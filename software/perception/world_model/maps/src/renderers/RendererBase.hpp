#ifndef _maps_RendererBase_hpp_
#define _maps_RendererBase_hpp_

#include <string>
#include <boost/shared_ptr.hpp>

// forward declarations
typedef struct _BotViewer BotViewer;
typedef struct _BotParam BotParam;
typedef struct _BotFrames BotFrames;
typedef struct _lcm_t lcm_t;
typedef struct _GdkEventKey GdkEventKey;
typedef struct _GdkEventButton GdkEventButton;
typedef struct _GdkEventMotion GdkEventMotion;
typedef struct _GdkEventScroll GdkEventScroll;
namespace Gtk {
  class Widget;
  class Container;
  class ToggleButton;
  class CheckButton;
  class SpinButton;
  class HScale;
  class ComboBox;
}
namespace lcm {
  class LCM;
  class Subscription;
}

namespace maps {

class RendererBase {
public:
  RendererBase(const std::string& iName,
               BotViewer* iViewer, const int iPriority,
               const lcm_t* iLcm,
               const BotParam* iParam, const BotFrames* iFrames);
  virtual ~RendererBase();

protected:
  // convenience methods for data access to subclasses
  std::string getName() const;
  boost::shared_ptr<lcm::LCM> getLcm() const;
  BotParam* getBotParam() const;
  BotFrames* getBotFrames() const;
  BotViewer* getBotViewer() const;
  Gtk::Container* getGtkContainer() const;
  Gtk::Widget* getGtkWidget(const std::string& iName) const;
  int64_t now() const;
  void requestDraw();

  // methods for registering commonly used objects
  void add(const lcm::Subscription* iSubscription);
  template<typename W, typename T>
  bool bind(W* iWidget, const std::string& iName, T& iData);

  // convenience methods to create, add, bind, and register common widgets
  Gtk::ToggleButton* addToggle(const std::string& iName, bool& iData);
  Gtk::CheckButton* addCheck(const std::string& iName, bool& iData);
  Gtk::SpinButton* addSpin(const std::string& iName, double& iData,
                           const double iMin, const double iMax,
                           const double iStep);
  Gtk::SpinButton* addSpin(const std::string& iName, int& iData,
                           const int iMin, const int iMax,
                           const int iStep);
  Gtk::HScale* addSlider(const std::string& iName, double& iData,
                         const double iMin, const double iMax,
                         const double iStep);
  Gtk::HScale* addSlider(const std::string& iName, int& iData,
                         const int iMin, const int iMax, const int iStep);
  Gtk::ComboBox* addCombo(const std::string& iName, int& iData,
                          const std::vector<std::string>& iLabels);
  Gtk::ComboBox* addCombo(const std::string& iName, int& iData,
                          const std::vector<std::string>& iLabels,
                          const std::vector<int>& iIndices);

  // event handling methods (can be overridden by subclasses)
  virtual double pickQuery(const double iRayStart[3],
                           const double iRayDir[3])        { return -1; }
  virtual double hoverQuery(const double iRayStart[3],
                            const double iRayDir[3])       { return -1; }
  virtual bool keyPress(const GdkEventKey* iEvent)         { return false; }
  virtual bool keyRelease(const GdkEventKey* iEvent)       { return false; }
  virtual bool mousePress(const GdkEventButton* iEvent,
                          const double iRayStart[3],
                          const double iRayDir[3])         { return false; }
  virtual bool mouseRelease(const GdkEventButton* iEvent,
                            const double iRayStart[3],
                            const double iRayDir[3])       { return false; }
  virtual bool mouseMotion(const GdkEventMotion* iEvent,
                           const double iRayStart[3],
                           const double iRayDir[3])        { return false; }
  virtual bool mouseScroll(const GdkEventScroll* iEvent,
                           const double iRayStart[3],
                           const double iRayDir[3])        { return false; }

  // actual rendering method (must be defined by subclasses)
  virtual void draw() = 0;


private:
  struct InternalHelper;
  boost::shared_ptr<InternalHelper> mHelper;
};

}

#endif
