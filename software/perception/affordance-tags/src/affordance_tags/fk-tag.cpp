// DRC-tags
// - detects tags (using april tags)
// - pairs detections with affordances in a library
// - updates the affordances in the affordances in the store
//
// TODO: read library from file
// TODO: add rate limiting (process is at about 15Hz)

// pre-sept: Tag36h11
// post sept: Tag16h5
//

#include <stdio.h>
#include <inttypes.h>
#include <iostream>
#include <Eigen/Dense>
#include <boost/shared_ptr.hpp>
#include <boost/assign/std/vector.hpp>

#include <lcm/lcm-cpp.hpp>
#include <bot_param/param_client.h>
#include <bot_frames_cpp/bot_frames_cpp.hpp>

#include <opencv2/opencv.hpp>
#include <image_io_utils/image_io_utils.hpp> // to simplify jpeg/zlib compression and decompression

#include <pronto_utils/pronto_vis.hpp> // visualize pt clds
#include <pronto_utils/pronto_lcm.hpp> // unpack lidar to xyz
#include <lcmtypes/bot_core.hpp>
#include <lcmtypes/drc/tag_detection_t.hpp>

#include <AprilTags/TagDetector.h>
#include <AprilTags/Tag16h5.h>

#include <ConciseArgs>

#include <affordance/AffordancePlusState.h>

using namespace std;
using namespace Eigen;
using namespace affordance;
using namespace boost::assign; // bring 'operator+()' into scope

// Details about a pairing between and tag and an affordance:
class AffTag {
public:
  AffTag(std::string name, double tag_size, bool active, AffPlusPtr aff_plus, KDL::Frame tag_to_aff ):
    name_ (name), tag_size_ (tag_size), active_(active), aff_plus_(aff_plus), tag_to_aff_(tag_to_aff){};
  std::string name_; // name of the object
  double tag_size_; // size of the tag
  bool active_; // has this object been seen yet
  AffPlusPtr aff_plus_; // details about the affordance
  KDL::Frame tag_to_aff_; // transform from the tag to the affordance
};

class Tags{
  public:
    Tags(boost::shared_ptr<lcm::LCM> &lcm_, bool verbose_, int64_t detection_period_);

    ~Tags(){
    }
  private:
    boost::shared_ptr<lcm::LCM> lcm_;
    bool verbose_;

    int64_t detection_period_;
    int64_t prev_detection_utime_;

    std::string camera_frame_, camera_channel_;
    int width_, height_;
    double fx_, fy_, cx_, cy_;

    void imageHandler(const lcm::ReceiveBuffer* rbuf, const std::string& channel, const  bot_core::image_t* msg);
    void multisenseHandler(const lcm::ReceiveBuffer* rbuf, const std::string& channel, const  bot_core::images_t* msg);

    void getTagList();
    void processTag(int64_t utime);//const  bot_core::image_t* msg);

    BotParam* botparam_;
    bot::frames* botframes_cpp_;

    pronto_vis* pc_vis_;
    pronto_lcm* pc_lcm_;
    image_io_utils*  imgutils_;
    uint8_t* img_buf_;

    AprilTags::TagDetector* tag_detector_;
    std::map<int,AffTag> tags_; // map of tag details
};

Tags::Tags(boost::shared_ptr<lcm::LCM> &lcm_, bool verbose_, int64_t detection_period_):
    lcm_(lcm_), verbose_(verbose_), detection_period_(detection_period_){
  botparam_ = bot_param_new_from_server(lcm_->getUnderlyingLCM(), 0);
  botframes_cpp_ = new bot::frames( lcm_ , botparam_ );

  camera_channel_ = "CAMERA";
  camera_frame_ = "CAMERA_LEFT";

  // subscribe but only keep a queue of one
  lcm::Subscription* sub1 = lcm_->subscribe( "CAMERA_LEFT" ,&Tags::imageHandler,this);
  sub1->setQueueCapacity(1);
  lcm::Subscription* sub = lcm_->subscribe( camera_channel_ ,&Tags::multisenseHandler,this);
  sub->setQueueCapacity(1);

  std::string left_str = "cameras."+camera_frame_+".intrinsic_cal";
  width_ = bot_param_get_int_or_fail(botparam_, (left_str+".width").c_str());
  height_ = bot_param_get_int_or_fail(botparam_,(left_str+".height").c_str());
  double vals[10];
  bot_param_get_double_array_or_fail(botparam_, (left_str+".pinhole").c_str(), vals, 5);
  fx_ = vals[0];
  fy_ = vals[1];
  cx_ = vals[3];
  cy_ = vals[4];

  // Vis Config:
  pc_vis_ = new pronto_vis( lcm_->getUnderlyingLCM() );
  // obj: id name type reset
  // pts: id name type reset objcoll usergb rgb
  pc_vis_->obj_cfg_list.push_back( obj_cfg(60000,"Tag Detection",5,1) );

  tag_detector_ = new AprilTags::TagDetector (AprilTags::tagCodes16h5);
  img_buf_= (uint8_t*) malloc(3* width_  *  height_);
  imgutils_ = new image_io_utils( lcm_->getUnderlyingLCM(), width_, height_); // extra space for stereo tasks

  Tags::getTagList();


  prev_detection_utime_ = 0;

}

void Tags::getTagList(){

  // TODO: read this information from a config file:
  // NB: uids count from 1
  // cylinder constructor: length, radius, mass, uid, map id, position
  // aff = 0 is reserved .... dont use it here

  AffPtr aff1(new AffordanceState());
  aff1->setToCylinder( 0.01, 0.28, 2,0, KDL::Frame(KDL::Vector(0,0,1)), Eigen::Vector3f(0,1,0));
  AffPlusPtr aff_plus1(new AffordancePlusState());
  aff_plus1->aff = aff1;
  KDL::Frame tag_to_aff1 = KDL::Frame(KDL::Rotation::RPY(0,0,0), KDL::Vector( 0, 0, 0));
  tags_.insert( make_pair( 6 , AffTag( "iRobot Left", 0.0492125, false, aff_plus1 ,tag_to_aff1 ) ) );

  AffPtr aff2(new AffordanceState());
  aff2->setToCylinder(0.122, 0.0325, 1,0, KDL::Frame(KDL::Vector(2.2,0,1)), Eigen::Vector3f(0,1,0));
  AffPlusPtr aff_plus2(new AffordancePlusState());
  aff_plus2->aff = aff2;
  KDL::Frame tag_to_aff2 = KDL::Frame(KDL::Rotation::RPY(0,0,0), KDL::Vector( 0, 0, -0.061));
  tags_.insert( make_pair( 2 , AffTag( "Drill Wall", 0.160338, false, aff_plus2 , tag_to_aff2 ) ) );

  AffPtr aff3(new AffordanceState());
  aff3->setToCylinder(0.122, 0.0325, 1,0, KDL::Frame(KDL::Vector(2,0,1)), Eigen::Vector3f(0,1,0));
  AffPlusPtr aff_plus3(new AffordancePlusState());
  aff_plus3->aff = aff3;
  KDL::Frame tag_to_aff3 = KDL::Frame(KDL::Rotation::RPY(0,0,0), KDL::Vector( 0, 0, -0.061));
  tags_.insert( make_pair( 3 , AffTag( "Can", 0.07778755, false, aff_plus3 , tag_to_aff3 ) ) );

}

// draw April tag detection on actual image
// NB: because the conversion was skipped R and B will be switched...
void draw_detection(cv::Mat& image, const AprilTags::TagDetection& detection) {
  // use corner points detected by line intersection
  std::pair<float, float> p1 = detection.p[0];
  std::pair<float, float> p2 = detection.p[1];
  std::pair<float, float> p3 = detection.p[2];
  std::pair<float, float> p4 = detection.p[3];
  // plot outline
  cv::line(image, cv::Point2f(p1.first, p1.second), cv::Point2f(p2.first, p2.second), cv::Scalar(255,0,0,0) );
  cv::line(image, cv::Point2f(p2.first, p2.second), cv::Point2f(p3.first, p3.second), cv::Scalar(0,255,0,0) );
  cv::line(image, cv::Point2f(p3.first, p3.second), cv::Point2f(p4.first, p4.second), cv::Scalar(0,0,255,0) );
  cv::line(image, cv::Point2f(p4.first, p4.second), cv::Point2f(p1.first, p1.second), cv::Scalar(255,0,255,0) );
  // mark center
  cv::circle(image, cv::Point2f(detection.cxy.first, detection.cxy.second), 8, cv::Scalar(0,0,255,0), 2);
  // print ID
  std::ostringstream strSt;
  strSt << "#" << detection.id;
  cv::putText(image, strSt.str(), cv::Point2f(detection.cxy.first + 10, detection.cxy.second + 10),
              cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0,0,255));
}


void Tags::imageHandler(const lcm::ReceiveBuffer* rbuf, const std::string& channel, const  bot_core::image_t* msg){
  imgutils_->decodeImageToRGB((msg),  img_buf_ );
  processTag(msg->utime);
}
void Tags::multisenseHandler(const lcm::ReceiveBuffer* rbuf, const std::string& channel, const  bot_core::images_t* msg){
  imgutils_->decodeImageToRGB(&(msg->images[0]),  img_buf_ );
  processTag(msg->utime);
}


void Tags::processTag(int64_t utime_in){
  int64_t elapsed_utime = utime_in - prev_detection_utime_;

  if (elapsed_utime < 0){
    std::cout << "Negative time, reset: " <<  elapsed_utime << "\n";
    prev_detection_utime_ = 0;
    elapsed_utime = utime_in - prev_detection_utime_;
  }

  if (elapsed_utime   <  detection_period_ ){
    std::cout << "Too recent: " <<  elapsed_utime << "\n";
    return;
  }
  prev_detection_utime_ = utime_in;


  // TODO: support jpeg compression

  cv::Mat img = cv::Mat::zeros(height_, width_,CV_8UC3);
  //memcpy(img.data,  msg->data.data() , width_*height_*3 );
  img.data = img_buf_;

  //cv::Mat img = cv::imdecode(cv::Mat(msg->images[0].data), -1);

  // I'm assuming incoming data has LCM RGB convention
  // to avoid color re-conversions, skip RGB2BGR conversion here:
  cv::Mat gray;
  cv::cvtColor(img, gray, CV_RGB2GRAY);
  std::vector<AprilTags::TagDetection> detections = tag_detector_->extractTags(gray); // find the tags!

  if (verbose_) cout << detections.size() << " tags detected:" << endl;
  std::vector < Isometry3dTime > detected_posesT;
  for (int i=0; i< detections.size(); i++) {
    cout << "  Id: " << detections[i].id << " -- " << "  Hamming distance: " << detections[i].hammingDistance << endl;

    // also highlight in the image
    if (verbose_) draw_detection(img, detections[i]);

    std::map<int, AffTag >::iterator it;
    it=tags_.find( detections[i].id );
    if(it!=tags_.end()) {
      // Get the relative transform, match with affordance and publish updated affordance:
      Eigen::Matrix4d T = detections[i].getRelativeTransform( it->second.tag_size_, fx_, fy_, cx_, cy_);
      Eigen::Isometry3d local_to_camera;
      botframes_cpp_->get_trans_with_utime( camera_frame_ , "local", utime_in, local_to_camera);
      Eigen::Isometry3d tag_pose = local_to_camera*Eigen::Isometry3d(T)   ;
      tag_pose.rotate( Eigen::Quaterniond(  euler_to_quat( 0 ,  (M_PI) ,  0 )  ) );

      if (verbose_){
        Isometry3dTime poseT = Isometry3dTime(utime_in + detections[i].id, tag_pose);
        detected_posesT.push_back( poseT);
        std::cout << "detected: " << detections[i].id << "|"
          << it->first << " is tag | "
          << it->second.name_ << " is name\n";
      }


      Eigen::Quaterniond r_x(tag_pose.rotation());

      bot_core::pose_t pose;
      pose.utime = utime_in;
      pose.pos[0] = tag_pose.translation().x();
      pose.pos[1] = tag_pose.translation().y();
      pose.pos[2] = tag_pose.translation().z();
      pose.orientation[0] =  r_x.w();
      pose.orientation[1] =  r_x.x();
      pose.orientation[2] =  r_x.y();
      pose.orientation[3] =  r_x.z();
      lcm_->publish("POSE_TAG", &pose );


      drc::tag_detection_t tag;
      tag.utime = utime_in;
      tag.pos[0] = tag_pose.translation().x();
      tag.pos[1] = tag_pose.translation().y();
      tag.pos[2] = tag_pose.translation().z();
      tag.orientation[0] =  r_x.w();
      tag.orientation[1] =  r_x.x();
      tag.orientation[2] =  r_x.y();
      tag.orientation[3] =  r_x.z();
      tag.id = detections[i].id;
      tag.cxy[0] = detections[i].cxy.first;
      tag.cxy[1] = detections[i].cxy.second;
      for (int jj=0; jj<4;jj++){
        tag.p[jj][0] = detections[i].p[jj].first;
        tag.p[jj][1] = detections[i].p[jj].second;
      }
      lcm_->publish("TAG_DETECTION", &tag );



    }
  }

  if (verbose_){
    pc_vis_->pose_collection_to_lcm_from_list(60000, detected_posesT);
    imgutils_->sendImage( img.data, utime_in, width_, height_, 3, "CAMERA_TAGS");
  }

}


int main( int argc, char** argv ){
  ConciseArgs parser(argc, argv, "drc-tags");
  bool verbose=FALSE;
  double period= 0.0001;
  parser.add(verbose, "v", "verbose", "Verbosity");
  parser.add(period, "p", "period", "[Min] period between detections in sec");
  parser.parse();
  cout << verbose << " is verbose\n";
  cout << period << " is period between detections\n";
  int64_t detection_period = period*1E6;
  cout << detection_period << " is period between detections [usec]\n";


  boost::shared_ptr<lcm::LCM> lcm(new lcm::LCM);
  if(!lcm->good()){
    std::cerr <<"ERROR: lcm is not good()" <<std::endl;
  }

  Tags app(lcm,verbose, detection_period);
  cout << "Ready to find tags" << endl << "============================" << endl;
  while(0 == lcm->handle());
  return 0;
}
