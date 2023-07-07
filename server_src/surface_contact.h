
#ifndef FORCE_SERVER_SURFACE_CONTACT_H
#define FORCE_SERVER_SURFACE_CONTACT_H

#include "open_haptics.h"

#include <minvr3.h>

#include "event_mgr.h"
#include "force_effect.h"


class SurfaceContact : public ForceEffect {
public:
    SurfaceContact(EventMgr* event_mgr);
    virtual ~SurfaceContact();

    const std::string name = "SurfaceContact";
    const std::string Name() const { return name; }

    // start/stop
    void OnStartEffect(VREvent* e);
    void OnStopEffect(VREvent* e);

    // force parameters
    void OnStiffnessChange(VREvent* e);
    void OnDampingChange(VREvent* e);
    void OnStaticFrictionChange(VREvent* e);
    void OnDynamicFrictionChange(VREvent* e);

    // geometery
    void OnBeginGeometry(VREvent* e);
    // call with VREventVector3 to add (x,y,z) to the vertex buffer
    void OnAddVertex(VREvent* e);
    // call with VREventVector3 to add a triangle formed by the three indices in the (x,y,z) data
    void OnAddIndices(VREvent* e);
    void OnEndGeometry(VREvent* e);

    // called by Phantom class
    void DrawHaptics();
    void DrawGraphics();
    void Reset();

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


