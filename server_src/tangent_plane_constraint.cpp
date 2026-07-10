#include "tangent_plane_constraint.h"
#include "force_messages.h"
#include "phantom.h"

TangentPlaneConstraint::TangentPlaneConstraint(EventMgr* event_mgr) :
    effect_id_(0), active_(false), has_valid_data_(false),
    stiffness_(0.8), damping_(0.1)
{
    std::string prefix = ForceMessages::get_force_effect_prefix() + Name() + "/";
    event_mgr->AddListener(prefix + "Start", this, &TangentPlaneConstraint::OnStartEffect);
    event_mgr->AddListener(prefix + "Stop", this, &TangentPlaneConstraint::OnStopEffect);
    event_mgr->AddListener(prefix + "SetContactData", this, &TangentPlaneConstraint::OnSetContactData);
    event_mgr->AddListener(prefix + "SetStiffness", this, &TangentPlaneConstraint::OnSetStiffness);
    event_mgr->AddListener(prefix + "SetDamping", this, &TangentPlaneConstraint::OnSetDamping);

    // Generate the custom effect ID
    effect_id_ = hlGenEffects(1);
}

TangentPlaneConstraint::~TangentPlaneConstraint() {
    if (hlIsEffect(effect_id_)) {
        hlDeleteEffects(effect_id_, 1);
    }
}

void TangentPlaneConstraint::OnStartEffect(VREvent* e) {
    if (!active_) {
        hlEffectd(HL_EFFECT_CALLBACK, (HLdouble)compute_tangent_force);
        hlEffectd(HL_EFFECT_USER_DATA, (HLdouble)this);
        hlStartEffect(HL_EFFECT_CUSTOM, effect_id_);
        active_ = true;
    }
}

void TangentPlaneConstraint::OnStopEffect(VREvent* e) {
    if (active_) {
        hlStopEffect(effect_id_);
        active_ = false;
    }
}

void TangentPlaneConstraint::OnSetContactData(VREvent* e) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    contact_point_[0] = e->get_data<float>("contactPointX");
    contact_point_[1] = e->get_data<float>("contactPointY");
    contact_point_[2] = e->get_data<float>("contactPointZ");

    surface_normal_[0] = e->get_data<float>("surfaceNormalX");
    surface_normal_[1] = e->get_data<float>("surfaceNormalY");
    surface_normal_[2] = e->get_data<float>("surfaceNormalZ");
    has_valid_data_ = true;
}

void TangentPlaneConstraint::OnSetStiffness(VREvent* e) {
    stiffness_ = e->get_data<float>("value");
}

void TangentPlaneConstraint::OnSetDamping(VREvent* e) {
    damping_ = e->get_data<float>("value");
}

void TangentPlaneConstraint::Reset() {
    if (active_) {
        hlStopEffect(effect_id_);
    }
    active_ = false;
    has_valid_data_ = false;
}

void TangentPlaneConstraint::DrawHaptics() {
    // This is intentionally empty. All force rendering is handled by the custom callback.
}

void TangentPlaneConstraint::DrawGraphics() {
    if (!active_ || !has_valid_data_) {
        return;
    }

    // Safely copy data for rendering
    hduVector3Dd point, normal;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        point = contact_point_;
        normal = surface_normal_;
    }

    // Draw a small quad to represent the tangent plane
    glDisable(GL_LIGHTING);
    glColor3f(0.2, 0.8, 0.2);

    hduVector3Dd u, v;
    hduVec3Dd_orthogonalVectors(normal, u, v);

    double size = 20.0; // 20mm
    hduVector3Dd p1 = point - u * size - v * size;
    hduVector3Dd p2 = point + u * size - v * size;
    hduVector3Dd p3 = point + u * size + v * size;
    hduVector3Dd p4 = point - u * size + v * size;

    glBegin(GL_QUADS);
    glVertex3dv(p1);
    glVertex3dv(p2);
    glVertex3dv(p3);
    glVertex3dv(p4);
    glEnd();

    // Draw the normal vector
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex3dv(point);
    glVertex3dv(point + normal * size * 2.0);
    glEnd();

    glEnable(GL_LIGHTING);
}

HLCALLBACK TangentPlaneConstraint::compute_tangent_force(const HDdouble force[3], const HDdouble position[3], const HDdouble velocity[3], HDdouble xform[16], HDdouble result[3], void* userdata)
{
    TangentPlaneConstraint* self = static_cast<TangentPlaneConstraint*>(userdata);
    result[0] = 0.0;
    result[1] = 0.0;
    result[2] = 0.0;

    if (!self || !self->has_valid_data_) {
        return;
    }

    // Safely copy data for calculation
    hduVector3Dd contact_point, surface_normal;
    {
        std::lock_guard<std::mutex> lock(self->data_mutex_);
        contact_point = self->contact_point_;
        surface_normal = self->surface_normal_;
    }

    hduVector3Dd stylus_pos(position);
    hduVector3Dd stylus_to_plane = stylus_pos - contact_point;

    // Calculate penetration depth
    double depth = stylus_to_plane.dot(surface_normal);

    if (depth < 0.0) {
        hduVector3Dd stylus_vel(velocity);
        double normal_velocity = stylus_vel.dot(surface_normal);

        // Spring-damper model: F = -k*x - c*v
        double force_magnitude = (-self->stiffness_ * depth) - (self->damping_ * normal_velocity);
        hduVector3Dd force_vec = force_magnitude * surface_normal;

        result[0] = force_vec[0];
        result[1] = force_vec[1];
        result[2] = force_vec[2];
    }
}