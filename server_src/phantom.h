#ifndef FORCE_SERVER_PHANTOM_H
#define FORCE_SERVER_PHANTOM_H

#include "open_haptics.h"

#include <map>
#include <string>

#include <minvr3.h>

#include "force_effect.h"

class EventMgr;

class Phantom {
public:
    Phantom(EventMgr* event_mgr);
    virtual ~Phantom();

    bool Init(const std::string &device_name = "Default PHANToM");
    
    // These functions should be called before the main rendering loop starts
    void RegisterForceEffect(const std::string& effect_name, ForceEffect* effect);

    void PollForInput();
    void UpdateHapticWorkspace();


    void BeginHapticFrame();
    void OnStartForceEffect(VREvent* event);
    void OnStopForceEffect(VREvent* event);
    void StopAllEffects();
    void DrawHaptics();
    void EndHapticFrame();

    // This should also be called once per frame from the main rendering loop.
    // Since it renders graphics only, it can safely be called within or outside
    // of the Begin/EndHapticFrame() pair. 
    void DrawGraphics();

    // Prints an error message and returns true if an error occurred, false is everything is ok
    static bool CheckHapticError();

    bool is_primary_btn_down();
    double* transform();
    double* position();
    double* rotation();
    
protected:
    EventMgr* event_mgr_;
    HHD hd_device_;
    HHLRC hl_context_;

    HDdouble transform_[16];
    HDdouble position_[3];
    HDdouble rotation_[4];
    
    std::map<std::string, ForceEffect*> effects_;
};


#endif
