

#ifndef FORCE_SERVER_POINT_CONSTRAINT_H
#define FORCE_SERVER_POINT_CONSTRAINT_H

#include "open_haptics.h"

#include <minvr3.h>

#include "event_mgr.h"
#include "force_effect.h"


class PointConstraint : public ForceEffect {
public:
    PointConstraint(EventMgr* event_mgr);
    virtual ~PointConstraint();

    const std::string name = "PointConstraint";
    const std::string Name() const { return name; }
        
    void OnStartEffect();
    void OnStopEffect();

    void OnPointChange(VREvent* e);
    void OnStiffnessChange(VREvent* e);
    void OnDampingChange(VREvent* e);
    void OnStaticFrictionChange(VREvent* e);
    void OnDynamicFrictionChange(VREvent* e);
    void OnSnapDistanceChange(VREvent* e);
    void OnSnapForceChange(VREvent* e);
    
    void DrawHaptics();
    void DrawGraphics();

private:
    
    bool initialized_;
    HLuint shape_id_;
    
    HLfloat point_[3];
    HLfloat stiffness_;
    HLfloat damping_;
    HLfloat static_friction_;
    HLfloat dynamic_friction_;
    HLfloat snap_dist_;
};

#endif


