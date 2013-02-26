#ifndef _maps_Types_hpp_
#define _maps_Types_hpp_

#include <pcl/point_types.h>
#include <Eigen/Geometry>

// forward declaration
namespace octomap {
  class OcTree;
}
namespace lcm {
  class LCM;
}
namespace pcl {
  class RangeImage;
}

namespace maps {
  typedef pcl::PointXYZRGB PointType;
  typedef pcl::PointCloud<PointType> PointCloud;

  typedef boost::shared_ptr<lcm::LCM> LcmPtr;

  struct PointSet {
    int64_t mTimestamp;
    float mMaxRange;
    PointCloud::Ptr mCloud;
  };

  struct Octree {
    boost::shared_ptr<octomap::OcTree> mTree;
    Eigen::Isometry3f mTransform;  // from map to reference coords
  };

  struct RangeImage {
    boost::shared_ptr<pcl::RangeImage> mImage;
    bool mOrthographic;
  };

}

#endif
