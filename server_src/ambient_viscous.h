

#ifndef FORCE_SERVER_AMBIENT_VISCOUS_H
#define FORCE_SERVER_AMBIENT_VISCOUS_H

#include <minvr3.h>

// OpenHaptics includes
#include <HD/hd.h>
#include <HL/hl.h>


#include "event_mgr.h"
#include "force_effect.h"

class AmbientViscous : public ForceEffect {
public:
    AmbientViscous(EventMgr* event_mgr);
    virtual ~AmbientViscous();

    const std::string name = "AmbientViscous";
    const std::string Name() const { return name; }
        
    void OnStartEffect(VREvent* e);
    void OnStopEffect(VREvent* e);

    void OnGainChange(VREvent* e);
    void OnMagnitudeCapChange(VREvent* e);
 
    void DrawHaptics();
    void DrawGraphics();
    void Reset();

private:
    HLuint effect_id_;

    bool params_dirty_; // true when any of the params below has changed; changes are applied to OpenHaptics during DrawHaptics()
    HLdouble gain_;
    HLdouble magnitude_cap_;

    bool active_;
    bool active_buffered_; // set by incoming events; acts as a dirty flag; if != active_ then the new state is applied during DrawHaptics()
};

#endif
