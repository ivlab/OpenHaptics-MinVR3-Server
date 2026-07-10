#ifndef TANGENT_PLANE_CONSTRAINT_H
#define TANGENT_PLANE_CONSTRAINT_H

#include "open_haptics.h"
#include <minvr3.h>
#include <mutex>

#include "event_mgr.hh"
#include "force_effect.h"

class TangentPlaneConstraint : public ForceEffect {
public:
    TangentPlaneConstraint(EventMgr* event_mgr);
    virtual ~TangentPlaneConstraint();

    const std::string name = "TangentPlaneConstraint";
    const std::string Name() const { return name; }

    // Event handlers
    void OnStartEffect(VREvent* e);
    void OnStopEffect(VREvent* e);
    void OnSetContactData(VREvent* e);
    void OnSetStiffness(VREvent* e);
    void OnSetDamping(VREvent* e);

    // Main methods
    void DrawHaptics() override;
    void DrawGraphics() override;
    void Reset() override;

private:
    // The static callback function for the custom force effect
    static HLCALLBACK compute_tangent_force(const HDdouble force[3], const HDdouble position[3], const HDdouble velocity[3], HDdouble xform[16], HDdouble result[3], void* userdata);

    HLuint effect_id_;
    bool active_;

    // Thread-safe state for contact data
    std::mutex data_mutex_;
    hduVector3Dd contact_point_;
    hduVector3Dd surface_normal_;
    bool has_valid_data_;

    // Haptic parameters
    HLdouble stiffness_;
    HLdouble damping_;
};

#endif // TANGENT_PLANE_CONSTRAINT_H