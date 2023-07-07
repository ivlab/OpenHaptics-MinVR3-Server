
#ifndef FORCE_SERVER_LINE_CONSTRAINT_H
#define FORCE_SERVER_LINE_CONSTRAINT_H

#include "open_haptics.h"

#include <minvr3.h>

#include "event_mgr.h"
#include "force_effect.h"


class LineConstraint : public ForceEffect {
public:
    LineConstraint(EventMgr* event_mgr);
    virtual ~LineConstraint();

    const std::string name = "LineConstraint";
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
    void OnBeginLines(VREvent* e);
    void OnAddVertex(VREvent* e);
    void OnEndLines(VREvent* e);

    // called by Phantom once per frame
    void DrawHaptics();
    void DrawGraphics();

private:
    bool active_;
    HLuint shape_id_;
    
    std::vector<hduVector3Df> verts_;
    std::vector<hduVector3Df> verts_tmp_buffer_;
    HLfloat stiffness_;
    HLfloat damping_;
    HLfloat static_friction_;
    HLfloat dynamic_friction_;
    HLfloat snap_dist_;
};

#endif


