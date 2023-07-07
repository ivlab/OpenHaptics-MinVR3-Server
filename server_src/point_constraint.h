

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
    
    // start/stop
    void OnStartEffect(VREvent* e);
    void OnStopEffect(VREvent* e);

    // force parameters
    void OnStiffnessChange(VREvent* e);
    void OnDampingChange(VREvent* e);
    void OnStaticFrictionChange(VREvent* e);
    void OnDynamicFrictionChange(VREvent* e);
    void OnSnapDistanceChange(VREvent* e);
    void OnSnapForceChange(VREvent* e);

    // geometery
    void OnBeginPoints(VREvent* e);
    void OnAddVertex(VREvent* e);
    void OnEndPoints(VREvent* e);

    // called by Phantom class
    void DrawHaptics();
    void DrawGraphics();
    void Reset();

private:    
    bool active_;
    HLuint shape_id_;
    
    std::vector<hduVector3Df> points_;
    std::vector<hduVector3Df> points_tmp_buffer_;
    HLfloat stiffness_;
    HLfloat damping_;
    HLfloat static_friction_;
    HLfloat dynamic_friction_;
    HLfloat snap_dist_;
};

#endif


