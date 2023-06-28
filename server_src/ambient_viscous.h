

#ifndef FORCE_SERVER_AMBIENT_VISCOUS_H
#define FORCE_SERVER_AMBIENT_VISCOUS_H

// OpenHaptics includes
#include <HD/hd.h>
#include <HL/hl.h>

#include <minvr3.h>

#include "event_mgr.h"
#include "force_effect.h"

class AmbientViscous : public ForceEffect {
public:
    AmbientViscous(EventMgr* event_mgr);
    virtual ~AmbientViscous();

    const std::string name = "AmbientViscous";
    const std::string Name() const { return name; }
        
    void Init();
    void OnStartEffect();
    void OnStopEffect();

    void OnGainChange(VREvent* e);
    void OnMagnitudeCapChange(VREvent* e);
 
    void DrawHaptics();
    void DrawGraphics();

private:
    
    HLuint effect_id_;
    HLdouble gain_;
    HLdouble magnitude_cap_;
};

#endif



