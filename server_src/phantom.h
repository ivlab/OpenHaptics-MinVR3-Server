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
    
    bool Init(const std::string &device_name = "Default PHANToM");
    void Reset();
    
    void PollForInput();
    void Draw();

    bool is_primary_btn_down();
    double* transform();
    double* position();
    double* rotation();
    
    void RegisterForceEffect(const std::string &effect_name, ForceEffect *effect);

    void OnStartForceEffect(VREvent* event);
    void OnStopForceEffect(VREvent* event);
    
protected:
    EventMgr* event_mgr_;
    HHD hd_device_;
    HHLRC hl_context_;

    HDdouble transform_[16];
    HDdouble position_[3];
    HDdouble rotation_[4];
    
    std::map<std::string, ForceEffect*> effects_;
    std::map<std::string, ForceEffect*> active_effects_;
};


#endif
