#ifndef _maps_SensorDataReceiver_hpp_
#define _maps_SensorDataReceiver_hpp_

#include <string>
#include <boost/shared_ptr.hpp>

namespace lcm { class LCM; }

namespace maps {

struct PointSet;

class SensorDataReceiver {
public:
  enum SensorType {
    SensorTypePlanarLidar,
    SensorTypePointCloud
  };

  struct SensorData {
    boost::shared_ptr<PointSet> mPointSet;
    SensorType mSensorType;
    std::string mChannel;
  };

public:
  SensorDataReceiver();
  ~SensorDataReceiver();

  void setLcm(const boost::shared_ptr<lcm::LCM>& iLcm);
  bool addChannel(const std::string& iSensorChannel,
                  const SensorType iSensorType,
                  const std::string& iTransformFrom,
                  const std::string& iTransformTo);
  void clearChannels();
  bool removeChannel(const std::string& iSensorChannel);

  void setMaxBufferSize(const int iSize);
  bool pop(SensorData& oData);
  bool waitForData(SensorData& oData);
  void unblock();

  bool start();
  bool stop();

protected:
  struct Helper;
  boost::shared_ptr<Helper> mHelper;
};

}

#endif
