

#ifndef FORCE_SERVER_AMBIENT_FRICTION_H
#define FORCE_SERVER_AMBIENT_FRICTION_H

#include <minvr3.h>

#include "event_mgr.h"
#include "force_effect.h"

class AmbientFriction : public ForceEffect {
public:
    AmbientFriction(EventMgr* event_mgr);
    virtual ~AmbientFriction();

    const std::string name = "AmbientFriction";
    const std::string Name() const { return name; }
        
    void Init();
    void OnStartEffect();
    void OnStopEffect();
    void OnParamChange(VREvent* e);

    void DrawHaptics();
    void DrawGraphics();

private:
    HLuint effect_id_;
    HLdouble gain_;
    HLdouble magnitude_cap_;
};

#endif


