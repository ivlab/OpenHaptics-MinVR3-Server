
#ifndef FORCE_SERVER_SURFACE_CONSTRAINT_H
#define FORCE_SERVER_SURFACE_CONSTRAINT_H

#include "open_haptics.h"

#include <minvr3.h>

#include "event_mgr.h"
#include "force_effect.h"


class SurfaceConstraint : public ForceEffect {
public:
    SurfaceConstraint(EventMgr* event_mgr);
    virtual ~SurfaceConstraint();

    const std::string name = "SurfaceConstraint";
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
    void OnBeginGeometry(VREvent* e);
    // call with VREventVector3 to add (x,y,z) to the vertex buffer
    void OnAddVertex(VREvent* e);
    // call with VREventVector3 to add a triangle formed by the three indices in the (x,y,z) data
    void OnAddIndices(VREvent* e);
    void OnEndGeometry(VREvent* e);

    // called by Phantom once per frame
    void DrawHaptics();
    void DrawGraphics();

private:
    bool active_;
    HLuint shape_id_;
    
    std::vector<hduVector3Df> verts_;
    std::vector<hduVector3Df> verts_tmp_buffer_;
    
    std::vector<int> indices_;
    std::vector<int> indices_tmp_buffer_;
    
    HLfloat stiffness_;
    HLfloat damping_;
    HLfloat static_friction_;
    HLfloat dynamic_friction_;
    HLfloat snap_dist_;
};

#endif


