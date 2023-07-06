

#ifndef FORCE_SERVER_AMBIENT_FRICTION_H
#define FORCE_SERVER_AMBIENT_FRICTION_H

#include <minvr3.h>

// OpenHaptics includes
#include <HD/hd.h>
#include <HL/hl.h>


#include "event_mgr.h"
#include "force_effect.h"

class AmbientFriction : public ForceEffect {
public:
    AmbientFriction(EventMgr* event_mgr);
    virtual ~AmbientFriction();

    const std::string name = "AmbientFriction";
    const std::string Name() const { return name; }
        
    void OnStartEffect();
    void OnStopEffect();

    void OnGainChange(VREvent* e);
    void OnMagnitudeCapChange(VREvent* e);
 
    void DrawHaptics();
    void DrawGraphics();

private:
    bool start_this_frame_;
    bool stop_this_frame_;
    bool update_this_frame_;
    bool active_;

    HLuint effect_id_;
    HLdouble gain_;
    HLdouble magnitude_cap_;
};

#endif


