#include "tangent_plane_constraint.h"
#include "force_messages.h"
#include "phantom.h"
#include <GL/gl.h> // Ensure appropriate OpenGL headers are present
#include <cmath>

// Helper function to calculate two unit vectors orthogonal to a normal vector
void calculateOrthogonalVectors(const hduVector3Dd& normal, hduVector3Dd& u, hduVector3Dd& v) {
    hduVector3Dd n = normal;
    n.normalize();

    // Select an axis that is not collinear with the normal vector
    // 0.577 is roughly 1/sqrt(3).
    hduVector3Dd axis;
    if (std::abs(n[0]) < 0.577) {
        axis = hduVector3Dd(1.0, 0.0, 0.0);
    }
    else if (std::abs(n[1]) < 0.577) {
        axis = hduVector3Dd(0.0, 1.0, 0.0);
    }
    else {
        axis = hduVector3Dd(0.0, 0.0, 1.0);
    }

    // Generate orthogonal coordinate frame via cross products
    u = n.crossProduct(axis);
    u.normalize();

    v = n.crossProduct(u);
    v.normalize();
}

TangentPlaneConstraint::TangentPlaneConstraint(EventMgr* event_mgr) :
    effect_id_(0), active_(false), has_valid_point_(false), has_valid_normal_(false),
    stiffness_(0.8), damping_(0.1)
{
    std::string prefix = ForceMessages::get_force_effect_prefix() + Name() + "/";
    event_mgr->AddListener(prefix + "Start", this, &TangentPlaneConstraint::OnStartEffect);
    event_mgr->AddListener(prefix + "Stop", this, &TangentPlaneConstraint::OnStopEffect);
    event_mgr->AddListener(prefix + "SetContactPoint", this, &TangentPlaneConstraint::OnSetContactPoint);
    event_mgr->AddListener(prefix + "SetSurfaceNormal", this, &TangentPlaneConstraint::OnSetSurfaceNormal);
    event_mgr->AddListener(prefix + "SetStiffness", this, &TangentPlaneConstraint::OnSetStiffness);
    event_mgr->AddListener(prefix + "SetDamping", this, &TangentPlaneConstraint::OnSetDamping);

    // Generate the custom HLAPI effect identifier
    effect_id_ = hlGenEffects(1);
    hlCallback(HL_EFFECT_COMPUTE_FORCE, (HLcallbackProc)compute_tangent_force, this);
}

TangentPlaneConstraint::~TangentPlaneConstraint() {
    if (hlIsEffect(effect_id_)) {
        hlDeleteEffects(effect_id_, 1);
    }
}

void TangentPlaneConstraint::OnStartEffect(VREvent* e) {
    if (!active_) {
        hlStartEffect(HL_EFFECT_CALLBACK, effect_id_);
        active_ = true;
    }
}

void TangentPlaneConstraint::OnStopEffect(VREvent* e) {
    if (active_) {
        hlStopEffect(effect_id_);
        active_ = false;
    }
}

void TangentPlaneConstraint::OnSetContactPoint(VREvent* e) {
    VREventVector3* e_p = dynamic_cast<VREventVector3*>(e);
    if (e_p != NULL) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        contact_point_[0] = e_p->x();
        contact_point_[1] = e_p->y();
        contact_point_[2] = e_p->z();
        has_valid_point_ = true;
    }
}

void TangentPlaneConstraint::OnSetSurfaceNormal(VREvent* e) {
    VREventVector3* e_p = dynamic_cast<VREventVector3*>(e);
    if (e_p != NULL) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        surface_normal_[0] = e_p->x();
        surface_normal_[1] = e_p->y();
        surface_normal_[2] = e_p->z();
        has_valid_normal_ = true;
    }
}

void TangentPlaneConstraint::OnSetStiffness(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) {
        stiffness_ = e_float->get_data();
    }
}

void TangentPlaneConstraint::OnSetDamping(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) {
        damping_ = e_float->get_data();
    }
}

void TangentPlaneConstraint::Reset() {
    if (active_) {
        hlStopEffect(effect_id_);
    }
    active_ = false;
    has_valid_point_ = false;
    has_valid_normal_ = false;
}

void TangentPlaneConstraint::DrawHaptics() {
    // Intentionally empty. Forced calculations render completely via compute_tangent_force.
}

void TangentPlaneConstraint::DrawGraphics() {
    if (!active_ || !has_valid_point_ || !has_valid_normal_) {
        return;
    }

    // Safely sample geometry snapshots for graphic routines
    hduVector3Dd point, normal;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        point = contact_point_;
        normal = surface_normal_;
    }

    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.8f, 0.2f);

    hduVector3Dd u, v;
    calculateOrthogonalVectors(normal, u, v);

    double size = 20.0; // 20mm visual plane dimensions
    hduVector3Dd p1 = point - u * size - v * size;
    hduVector3Dd p2 = point + u * size - v * size;
    hduVector3Dd p3 = point + u * size + v * size;
    hduVector3Dd p4 = point - u * size + v * size;

    glBegin(GL_QUADS);
    glVertex3dv((double*)p1); // Fixed pointer typecast conversions
    glVertex3dv((double*)p2);
    glVertex3dv((double*)p3);
    glVertex3dv((double*)p4);
    glEnd();

    // Render surface direction pointer
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex3dv((double*)point);
    hduVector3Dd line_end = point + normal * size * 2.0;
    glVertex3dv((double*)line_end);
    glEnd();

    glEnable(GL_LIGHTING);
}

HLboolean HLCALLBACK TangentPlaneConstraint::compute_tangent_force(HLenum type, HLuint effect, HLdouble* force, void* userdata)
{
    // Initialize forces to 0
    force[0] = 0.0;
    force[1] = 0.0;
    force[2] = 0.0;

    TangentPlaneConstraint* self = static_cast<TangentPlaneConstraint*>(userdata);

    // Safeguard check to ensure data structures are initialized
    if (!self || !self->has_valid_point_ || !self->has_valid_normal_) {
        return HL_TRUE;
    }

    // Extract device metrics instantaneously inside the high-rate thread pipeline
    HDdouble devicePos[3];
    HDdouble deviceVel[3];
    hdGetDoublev(HD_CURRENT_POSITION, devicePos);
    hdGetDoublev(HD_CURRENT_VELOCITY, deviceVel);

    hduVector3Dd stylus_pos(devicePos);
    hduVector3Dd stylus_vel(deviceVel);

    // CRITICAL: Mutex locking is omitted here.
    // Reading data values directly avoids locking collisions inside the 1000Hz loop.
    hduVector3Dd contact_point = self->contact_point_;
    hduVector3Dd surface_normal = self->surface_normal_;

    hduVector3Dd stylus_to_plane = stylus_pos - contact_point;

    // Projection calculation tracking boundary invasion
    double depth = stylus_to_plane.dotProduct(surface_normal);

    // Penetration occurs if depth evaluates to less than 0
    if (depth < 0.0) {
        double normal_velocity = stylus_vel.dotProduct(surface_normal);

        // Spring-damper physics representation: F = -k*x - c*v
        double force_magnitude = (-self->stiffness_ * depth) - (self->damping_ * normal_velocity);
        hduVector3Dd force_vec = force_magnitude * surface_normal;

        force[0] = force_vec[0];
        force[1] = force_vec[1];
        force[2] = force_vec[2];
    }

    return HL_TRUE;
}
