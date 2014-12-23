// Do Vo, can occasionally use IMU to avoid orientation drift
// step one is

// the frames of interest are:
// pose body ... arbitrary position at root of frames tree, not actually updated


// pose_head ... inside the sensor, not a physical place
// pose camera ... actually using this as the navigation source via vo
// imu - not using its accelerations, only the orientation.

// i estimate motion of camera from t to t+1
// determine motion of head, via frames from t to t+1
//

#include <zlib.h>
#include <lcm/lcm-cpp.hpp>
#include <bot_param/param_client.h>
#include <bot_frames/bot_frames.h>
#include <bot_frames_cpp/bot_frames_cpp.hpp>

#include <lcmtypes/bot_core.hpp>
#include <lcmtypes/multisense.hpp>
#include <lcmtypes/microstrain_comm.hpp>

#include "drcvision/voconfig.hpp"
#include "drcvision/vofeatures.hpp"
#include "drcvision/voestimator.hpp"
#include "fovision.hpp"

#include <pronto_utils/pronto_vis.hpp> // visualize pt clds
#include <ConciseArgs>

/// For Forward Kinematics from body to head:
#include "urdf/model.h"
#include "kdl/tree.hpp"
#include "kdl_parser/kdl_parser.hpp"
#include "forward_kinematics/treefksolverposfull_recursive.hpp"
#include <model-client/model-client.hpp>

#include <image_io_utils/image_io_utils.hpp> // to simplify jpeg/zlib compression and decompression

#include <opencv/cv.h> // for disparity 

using namespace std;
using namespace cv; // for disparity ops

struct CommandLineConfig
{
  std::string camera_config; // which block from the cfg to read
  int fusion_mode;
  bool feature_analysis;
  std::string output_extension;
  bool output_signal;
  bool vicon_init; // initializae off of vicon
  std::string input_channel;
};

class StereoOdom{
  public:
    StereoOdom(boost::shared_ptr<lcm::LCM> &lcm_, const CommandLineConfig& cl_cfg_);
    
    ~StereoOdom(){
      free (left_buf_);
      free(right_buf_);
    }

  private:
    const CommandLineConfig cl_cfg_;    
    
    int image_size_; // just the resolution of the image
    uint8_t* left_buf_;
    uint8_t* right_buf_;
    mutable std::vector<float> disparity_buf_; // Is mutable necessary?
    uint8_t* rgb_buf_ ;
    uint8_t* decompress_disparity_buf_;    
    image_io_utils*  imgutils_;    
    
    int64_t utime_cur_;

    boost::shared_ptr<lcm::LCM> lcm_;
    BotParam* botparam_;
    BotFrames* botframes_;
    bot::frames* botframes_cpp_;
    voconfig::KmclConfiguration* config;

    //
    FoVision* vo_;

    //
    VoFeatures* features_;
    int64_t utime_prev_;
    uint8_t* left_buf_ref_; // copies of the reference images - probably can be extracted from fovis directly
    int64_t ref_utime_;
    Eigen::Isometry3d ref_camera_pose_; // [pose of the camera when the reference frames changed
    bool changed_ref_frames_;

    VoEstimator* estimator_;

    void featureAnalysis();
    void updateMotion();
    void unpack_multisense(const multisense::images_t *msg);
    void multisenseLDHandler(const lcm::ReceiveBuffer* rbuf, const std::string& channel, const  multisense::images_t* msg);   

    bool pose_initialized_;

    int imu_counter_;
    void microstrainHandler(const lcm::ReceiveBuffer* rbuf, const std::string& channel, const  microstrain::ins_t* msg);
    void fuseInterial(Eigen::Quaterniond imu_robotorientation,int correction_frequency, int64_t utime);
    
};    

StereoOdom::StereoOdom(boost::shared_ptr<lcm::LCM> &lcm_, const CommandLineConfig& cl_cfg_) : 
       lcm_(lcm_), cl_cfg_(cl_cfg_), utime_cur_(0), utime_prev_(0), 
       ref_utime_(0), changed_ref_frames_(false)
{
  botparam_ = bot_param_new_from_server(lcm_->getUnderlyingLCM(), 0);
  botframes_= bot_frames_get_global(lcm_->getUnderlyingLCM(), botparam_);
  botframes_cpp_ = new bot::frames(botframes_);
  
  // Read config from file:
  config = new voconfig::KmclConfiguration(botparam_, cl_cfg_.camera_config);

  boost::shared_ptr<fovis::StereoCalibration> stereo_calibration_;
  stereo_calibration_ = boost::shared_ptr<fovis::StereoCalibration>(config->load_stereo_calibration());
  image_size_ = stereo_calibration_->getWidth() * stereo_calibration_->getHeight();
  left_buf_ = (uint8_t*) malloc(3*image_size_);
  right_buf_ = (uint8_t*) malloc(3*image_size_);
  left_buf_ref_ = (uint8_t*) malloc(3*image_size_); // used of feature output 
  rgb_buf_ = (uint8_t*) malloc(10*image_size_ * sizeof(uint8_t)); 
  decompress_disparity_buf_ = (uint8_t*) malloc( 4*image_size_*sizeof(uint8_t));  // arbitary size chosen..
  
  vo_ = new FoVision(lcm_ , stereo_calibration_);
  features_ = new VoFeatures(lcm_, stereo_calibration_->getWidth(), stereo_calibration_->getHeight() );
  estimator_ = new VoEstimator(lcm_ , botframes_, cl_cfg_.output_extension );
  lcm_->subscribe( cl_cfg_.input_channel,&StereoOdom::multisenseLDHandler,this);

  Eigen::Isometry3d init_pose;
  init_pose = Eigen::Isometry3d::Identity();
  init_pose.translation().x() = 0;
  init_pose.translation().y() = 0;
  init_pose.translation().z() = 1.65; // nominal head height
  estimator_->setHeadPose(init_pose);

  // IMU:
  pose_initialized_=false;
  imu_counter_=0;
  lcm_->subscribe("MICROSTRAIN_INS",&StereoOdom::microstrainHandler,this);

  imgutils_ = new image_io_utils( lcm_->getUnderlyingLCM(), stereo_calibration_->getWidth(), 2*stereo_calibration_->getHeight()); // extra space for stereo tasks
  cout <<"StereoOdom Constructed\n";
}


int counter =0;
void StereoOdom::featureAnalysis(){

  /// Incremental Feature Output:
  if (counter%5 == 0 ){
    features_->setFeatures(vo_->getMatches(), vo_->getNumMatches() , utime_cur_);
    features_->setCurrentImage(left_buf_);
    //features_->setCurrentImages(left_buf_, right_buf_);
    features_->setCurrentCameraPose( estimator_->getCameraPose() );
    features_->doFeatureProcessing(1); // 1 = send the FEATURES_CUR
  }
  
  /// Reference Feature Output: ///////////////////////////////////////////////
  // Check we changed reference frame last iteration, if so output the set of matching inliers:
  if (changed_ref_frames_) {
    if (ref_utime_ > 0){ // skip the first null image
      if(vo_->getNumMatches() > 200){ // if less than 50 features - dont bother writing
      // was:      if(featuresA.size() > 50){ // if less than 50 features - dont bother writing
        cout << "ref frame from " << utime_prev_ << " at " << utime_cur_ <<  " with " <<vo_->getNumMatches()<<" matches\n";
        features_->setFeatures(vo_->getMatches(), vo_->getNumMatches() , ref_utime_);
        features_->setReferenceImage(left_buf_ref_);
        features_->setReferenceCameraPose( ref_camera_pose_ );
        features_->doFeatureProcessing(0); // 0 = send the FEATURES_REF
      }
    }
    changed_ref_frames_=false;
  }

  if (vo_->getChangeReferenceFrames()){ // If we change reference frame, note the change for the next iteration.
    ref_utime_ = utime_cur_;
    ref_camera_pose_ = estimator_->getCameraPose(); // publish this pose when the 
    // TODO: only copy gray data if its grey
    std::copy( left_buf_ , left_buf_ + 3*image_size_  , left_buf_ref_); // Keep the image buffer to write with the features:
    changed_ref_frames_=true;
  }
  counter++;
}

void StereoOdom::updateMotion(){
  Eigen::Isometry3d delta_camera;
  Eigen::MatrixXd delta_cov;
  fovis::MotionEstimateStatusCode delta_status;
  vo_->getMotion(delta_camera, delta_cov, delta_status );
  vo_->fovis_stats();
  
  estimator_->voUpdate(utime_cur_, delta_camera);
}

/// Added for RGB-to-Gray:
int pixel_convert_8u_rgb_to_8u_gray (uint8_t *dest, int dstride, int width,
        int height, const uint8_t *src, int sstride)
{
  int i, j;
  for (i=0; i<height; i++) {
    uint8_t *drow = dest + i * dstride;
    const uint8_t *srow = src + i * sstride;
    for (j=0; j<width; j++) {
      drow[j] = 0.2125 * srow[j*3+0] +
        0.7154 * srow[j*3+1] +
        0.0721 * srow[j*3+2];
    }
  }
  return 0;
}


void StereoOdom::unpack_multisense(const multisense::images_t *msg){
  if (msg->images[0].pixelformat == BOT_CORE_IMAGE_T_PIXEL_FORMAT_RGB ){
    rgb_buf_ = (uint8_t*) msg->images[0].data.data();
  }else if (msg->images[0].pixelformat == BOT_CORE_IMAGE_T_PIXEL_FORMAT_GRAY ){
    rgb_buf_ = (uint8_t*) msg->images[0].data.data();
  }else if (msg->images[0].pixelformat == BOT_CORE_IMAGE_T_PIXEL_FORMAT_MJPEG ){
    jpeg_decompress_8u_rgb ( msg->images[0].data.data(), msg->images[0].size,
        rgb_buf_, msg->images[0].width, msg->images[0].height, msg->images[0].width* 3);
    pixel_convert_8u_rgb_to_8u_gray(  left_buf_, msg->images[0].width, msg->images[0].width, msg->images[0].height, 
                                      rgb_buf_,  msg->images[0].width*3);
  }else{
    std::cout << "StereoOdom::unpack_multisense | image type not understood\n";
    exit(-1);
  }

  // TODO: support other modes (as in the renderer)
  if (msg->image_types[1] == multisense::images_t::DISPARITY_ZIPPED) {
    unsigned long dlen = msg->images[0].width*msg->images[0].height*2 ;//msg->depth.uncompressed_size;
    uncompress(decompress_disparity_buf_ , &dlen, msg->images[1].data.data(), msg->images[1].size);
  } else{
    std::cout << "StereoOdom::unpack_multisense | depth type not understood\n";
    exit(-1);
  }
}

void StereoOdom::multisenseLDHandler(const lcm::ReceiveBuffer* rbuf, 
     const std::string& channel, const  multisense::images_t* msg){
  int w = msg->images[0].width;
  int h = msg->images[0].height;

  utime_prev_ = utime_cur_;
  utime_cur_ = msg->utime;

  unpack_multisense(msg);
 
  // Convert Carnegie disparity format into floating point disparity. Store in local buffer
  Mat disparity_orig_temp = Mat::zeros(h,w,CV_16UC1); // h,w
  disparity_orig_temp.data = (uchar*) decompress_disparity_buf_;   // ... is a simple assignment possible?  
  cv::Mat_<float> disparity_orig(h, w);
  disparity_orig = disparity_orig_temp;
  disparity_buf_.resize(h * w);
  cv::Mat_<float> disparity(h, w, &(disparity_buf_[0]));
  disparity = disparity_orig / 16.0;
  
  vo_->doOdometry(left_buf_,disparity_buf_.data(), msg->utime );
  updateMotion();

  if(cl_cfg_.feature_analysis)
    featureAnalysis();
}


Eigen::Isometry3d KDLToEigen(KDL::Frame tf){
  Eigen::Isometry3d tf_out;
  tf_out.setIdentity();
  tf_out.translation()  << tf.p[0], tf.p[1], tf.p[2];
  Eigen::Quaterniond q;
  tf.M.GetQuaternion( q.x() , q.y(), q.z(), q.w());
  tf_out.rotate(q);
  return tf_out;
}



// Transform the Microstrain IMU orientation into the head frame:
// TODO: add an imu frame into the config
Eigen::Quaterniond microstrainIMUToRobotOrientation(const microstrain::ins_t *msg){
  Eigen::Quaterniond m(msg->quat[0],msg->quat[1],msg->quat[2],msg->quat[3]);
  Eigen::Isometry3d motion_estimate;
  motion_estimate.setIdentity();
  motion_estimate.translation() << 0,0,0;
  motion_estimate.rotate(m);

  // rotate coordinate frame so that look vector is +X, and up is +Z
  Eigen::Matrix3d M;

  //convert imu on bocelli cane from:
  //x+ right, z+ down, y+ back
  //to x+ forward, y+ left, z+ up (robotics)
  //M <<  0,  -1, 0,
  //    -1, 0, 0,
  //    0, 0, -1;

  //convert imu on drc rig from:
  //x+ back, z+ down, y+ left
  //to x+ forward, y+ left, z+ up (robotics)
  M <<  -1,  0, 0,
      0, 1, 0,
      0, 0, -1;

  motion_estimate= M * motion_estimate;
  Eigen::Vector3d translation(motion_estimate.translation());
  Eigen::Quaterniond rotation(motion_estimate.rotation());
  rotation = rotation * M.transpose();

  Eigen::Isometry3d motion_estimate_out;
  motion_estimate_out.setIdentity();
  motion_estimate_out.translation() << translation[0],translation[1],translation[2];
  motion_estimate_out.rotate(rotation);
  Eigen::Quaterniond r_x(motion_estimate_out.rotation());

  return r_x;
}

void StereoOdom::fuseInterial(Eigen::Quaterniond imu_robotorientation,
                              int correction_frequency, int64_t utime){
  if (cl_cfg_.fusion_mode==0){
    //    cout << "got IMU measurement - not incorporating them\n";
    return;
  }
  
  if (!pose_initialized_){
    if((cl_cfg_.fusion_mode ==1) ||(cl_cfg_.fusion_mode ==2) ){
      Eigen::Isometry3d init_pose;
      init_pose.setIdentity();
      init_pose.translation() << 0,0,0;
      init_pose.rotate( imu_robotorientation );
      estimator_->setHeadPose(init_pose);
      pose_initialized_ = true;
      cout << "got first IMU measurement\n";
      return;
    }
  }
  
  if((cl_cfg_.fusion_mode ==2) ||(cl_cfg_.fusion_mode ==3) ){
    if (1==1){//imu_counter_== correction_frequency){
      // Every X frames: replace the pitch and roll with that from the IMU
      // convert the camera pose to body frame
      // extract xyz and yaw from body frame
      // extract pitch and roll from imu (in body frame)
      // combine, convert to camera frame... set as pose
      bool verbose = false;
      
      Eigen::Isometry3d local_to_head = estimator_->getHeadPose();// _local_to_camera *cam2head;
      std::stringstream ss2;
      print_Isometry3d(local_to_head, ss2);
      double rpy[3];
      quat_to_euler(  Eigen::Quaterniond(local_to_head.rotation()) , rpy[0], rpy[1], rpy[2]);
      
      if (verbose){
        std::cout << "local_to_head: " << ss2.str() << " | "<< 
          rpy[0]*180/M_PI << " " << rpy[1]*180/M_PI << " " << rpy[2]*180/M_PI << "\n";        
      }
        
      double rpy_imu[3];
      quat_to_euler( imu_robotorientation , 
                      rpy_imu[0], rpy_imu[1], rpy_imu[2]);
      if (verbose){
        std::cout <<  rpy_imu[0]*180/M_PI << " " << rpy_imu[1]*180/M_PI << " " << rpy_imu[2]*180/M_PI << " rpy_imu\n";        
        cout << "IMU correction | roll pitch | was: "
            << rpy[0]*180/M_PI << " " << rpy[1]*180/M_PI << " | now: "
            << rpy_imu[0]*180/M_PI << " " << rpy_imu[1]*180/M_PI << "\n";
      }
      
      
      // NBNBNBNBNB was: // pitch and roll only:
      //Eigen::Quaterniond revised_local_to_head_quat = euler_to_quat( ypr[0], ypr_imu[1], ypr_imu[2]);             
      // ypr:
      Eigen::Quaterniond revised_local_to_head_quat = imu_robotorientation;
      ///////////////////////////////////////
      Eigen::Isometry3d revised_local_to_head;
      revised_local_to_head.setIdentity();
      revised_local_to_head.translation() = local_to_head.translation();
      revised_local_to_head.rotate(revised_local_to_head_quat);
      
      if (verbose){
        std::stringstream ss4;
        print_Isometry3d(revised_local_to_head, ss4);
        quat_to_euler(  Eigen::Quaterniond(revised_local_to_head.rotation()) , rpy[0], rpy[1], rpy[2]);
        std::cout << "local_revhead: " << ss4.str() << " | "<< 
          rpy[0]*180/M_PI << " " << rpy[1]*180/M_PI << " " << rpy[2]*180/M_PI << "\n";        
      }
      estimator_->setHeadPose(revised_local_to_head);

      // this is not correct, but ok for now:
      estimator_->publishUpdate(utime, revised_local_to_head, revised_local_to_head);
    }
    if (imu_counter_ > correction_frequency) { imu_counter_ =0; }
    imu_counter_++;
  }

}

void StereoOdom::microstrainHandler(const lcm::ReceiveBuffer* rbuf, 
     const std::string& channel, const  microstrain::ins_t* msg){
  Eigen::Quaterniond imu_robotorientation;
  imu_robotorientation =microstrainIMUToRobotOrientation(msg);
  int correction_frequency=100;
  fuseInterial(imu_robotorientation, correction_frequency, msg->utime);
}


int main(int argc, char **argv){
  CommandLineConfig cl_cfg;
  cl_cfg.camera_config = "CAMERA";
  cl_cfg.input_channel = "CAMERA_BLACKENED";
  cl_cfg.output_signal = FALSE;
  cl_cfg.feature_analysis = FALSE; 
  cl_cfg.vicon_init = FALSE;
  cl_cfg.fusion_mode = 0;
  cl_cfg.output_extension = "";

  ConciseArgs parser(argc, argv, "simple-fusion");
  parser.add(cl_cfg.camera_config, "c", "camera_config", "Camera Config block to use: CAMERA, stereo, stereo_with_letterbox");
  parser.add(cl_cfg.output_signal, "p", "output_signal", "Output POSE_BODY and POSE_BODY_ALT signals");
  parser.add(cl_cfg.feature_analysis, "f", "feature_analysis", "Publish Feature Analysis Data");
  parser.add(cl_cfg.vicon_init, "v", "vicon_init", "Bootstrap internal estimate using VICON_FRONTPLATE");
  parser.add(cl_cfg.fusion_mode, "m", "fusion_mode", "0 none, 1 at init, 2 every second, 3 init from gt, then every second");
  parser.add(cl_cfg.input_channel, "i", "input_channel", "input_channel - CAMERA or CAMERA_BLACKENED");
  parser.add(cl_cfg.output_extension, "o", "output_extension", "Extension to pose channels (e.g. '_VO' ");
  parser.parse();
  cout << cl_cfg.fusion_mode << " is fusion_mode\n";
  cout << cl_cfg.camera_config << " is camera_config\n";
  
  
  boost::shared_ptr<lcm::LCM> lcm(new lcm::LCM);
  if(!lcm->good()){
    std::cerr <<"ERROR: lcm is not good()" <<std::endl;
  }
  StereoOdom fo= StereoOdom(lcm, cl_cfg);
  while(0 == lcm->handle());
}
