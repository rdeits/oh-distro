#ifndef KEYBOARD_SIGNAL_UTILS_HPP
#define KEYBOARD_SIGNAL_UTILS_HPP

#include <iostream>
#include <boost/signals2.hpp> 
#include <boost/function.hpp>
#include <boost/bind.hpp>

using namespace std;
namespace visualization_utils {

  // control keys
  //65505 Shift(left)   65506 Shift(right)
  //65507 Control(left)   65508 Control(right)  
  //65507 Control(left)   65508 Control(right)  
  #define SHIFT_L 65505
  #define SHIFT_R 65506
  #define CNTRL_L 65507
  #define CNTRL_R 65508
  #define ALT_L   65513
  #define ALT_R   65514
  #define BKSPACE 65288
  #define LFTARRW 65361
  #define UPARRW  65362
  #define RGTARRW 65363
  #define DWNARRW 65364
  #define F1 65470
  #define F2 65471
  #define F3 65472
  #define F4 65473
  #define F5 65474
  #define F6 65475
  #define F7 65476
  #define F8 65477
  #define F9 65478
  #define F10 65479
  #define F11 65480
  #define F12 65481
  #define ESC 65307
  
  // A global keyboard signal with the keyval integer and a boolean 
  // indicating if it's a key-press or a key-release.
  typedef boost::signals2::signal<void(int,bool)> KeyboardSignal; 
  typedef boost::shared_ptr<KeyboardSignal> KeyboardSignalRef;

    // should this be interface class KeyboardSignalHandler?
    class KeyboardSignalHandler 
    {
      public:
        KeyboardSignalHandler(KeyboardSignalRef signalRef, boost::function<void(int,bool)> func):_user_callback(func)
        {
          _connection =  signalRef->connect(boost::bind(&KeyboardSignalHandler::callback,this,_1,_2));
        };
        
        ~KeyboardSignalHandler(){
          _connection.disconnect();
        }

        void callback(int keyval, bool is_pressed)
        {
          /*if(is_pressed) 
          {
            cout << "KeyPress Event:  Keyval: " << keyval << endl;
          }
          else {
            cout << "KeyRelease Event: Keyval: " << keyval << endl;
          }*/
          if(!_user_callback.empty())
             _user_callback(keyval,is_pressed); 
          else
             cerr << "ERROR in visualization_utils::KeyboardSignalHandler:  user callback function reference is empty!" << endl;
        }
        
        private:
        boost::signals2::connection _connection; 
        boost::function<void (int,bool)> _user_callback; 
        
    };

} // end namespace 


#endif //KEYBOARD_SIGNAL_UTILS_HPP