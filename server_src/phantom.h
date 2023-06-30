#ifndef FORCE_SERVER_PHANTOM_H
#define FORCE_SERVER_PHANTOM_H

#include <minvr3.h>

// OpenHaptics includes
#include <HD/hd.h>
#include <HL/hl.h>

#include <map>
#include <string>

#include "force_effect.h"

class EventMgr;

class Phantom {
public:
    Phantom(EventMgr* event_mgr);
    virtual ~Phantom();

    // These functions should be called before the main rendering loop starts
    void RegisterForceEffect(const std::string& effect_name, ForceEffect* effect);
    bool Init(const std::string &device_name = "Default PHANToM");
    
    // These functions should be called from within the main rendering loop.
    // BeginHapticFrame() should be he first haptic function called each frame
    // and EndHapticFrame() should be the last.
    void BeginHapticFrame();
    void PollForInput();
    void OnStartForceEffect(VREvent* event);
    void OnStopForceEffect(VREvent* event);
    void DrawHaptics();
    void Reset();
    void EndHapticFrame();

    // This should also be called once per frame from the main rendering loop.
    // Since it renders graphics only, it can safely be called within or outside
    // of the Begin/EndHapticFrame() pair. 
    void DrawGraphics();

    // Prints an error message and returns true if an error occurred, false is everything is ok
    bool CheckHapticError();

    bool is_primary_btn_down();
    double* transform();
    double* position();
    double* rotation();
    
protected:
    EventMgr* event_mgr_;
    HHD hd_device_;
    HHLRC hl_context_;
    bool initialized_;

    HDdouble transform_[16];
    HDdouble position_[3];
    HDdouble rotation_[4];
    
    std::map<std::string, ForceEffect*> effects_;
    std::map<std::string, ForceEffect*> active_effects_;
};


#endif
