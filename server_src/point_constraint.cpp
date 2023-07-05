
#include "point_constraint.h"

#include <iostream>
#include "force_messages.h"


PointConstraint::PointConstraint(EventMgr* event_mgr) {
    point_[0] = 0.0f;         // x
    point_[1] = 0.0f;         // y
    point_[2] = 0.0f;         // z
    stiffness_ = 0.2f;        // 0..1
    damping_ = 0.2f;          // 0..1
    static_friction_ = 0.2f;  // 0..1
    dynamic_friction_ = 0.2f; // 0..1
    snap_dist_ = 10.0f;       // dist in mm

    std::string event_name = ForceMessages::get_force_effect_param_event_name(Name(), "Point");
    event_mgr->AddListener(event_name, this, &PointConstraint::OnPointChange);

    event_name = ForceMessages::get_force_effect_param_event_name(Name(), "Stiffness");
    event_mgr->AddListener(event_name, this, &PointConstraint::OnStiffnessChange);
    
    event_name = ForceMessages::get_force_effect_param_event_name(Name(), "Damping");
    event_mgr->AddListener(event_name, this, &PointConstraint::OnDampingChange);

    event_name = ForceMessages::get_force_effect_param_event_name(Name(), "StaticFriction");
    event_mgr->AddListener(event_name, this, &PointConstraint::OnStaticFrictionChange);
    
    event_name = ForceMessages::get_force_effect_param_event_name(Name(), "DynamicFriction");
    event_mgr->AddListener(event_name, this, &PointConstraint::OnDynamicFrictionChange);
    
    event_name = ForceMessages::get_force_effect_param_event_name(Name(), "SnapDistance");
    event_mgr->AddListener(event_name, this, &PointConstraint::OnSnapDistanceChange);
    
    event_name = ForceMessages::get_force_effect_param_event_name(Name(), "SnapForce");
    event_mgr->AddListener(event_name, this, &PointConstraint::OnSnapForceChange);
}

PointConstraint::~PointConstraint() {
}

void PointConstraint::OnPointChange(VREvent* e) {
    VREventVector3* e_p = dynamic_cast<VREventVector3*>(e);
    if (e_p != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        point_[0] = e_p->x();
        point_[1] = e_p->y();
        point_[2] = e_p->z();
    }
}

void PointConstraint::OnStiffnessChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        stiffness_ = e_float->get_data();
    }
}

void PointConstraint::OnDampingChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        damping_ = e_float->get_data();
    }
}

void PointConstraint::OnStaticFrictionChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        static_friction_ = e_float->get_data();
    }
}

void PointConstraint::OnDynamicFrictionChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        dynamic_friction_ = e_float->get_data();
    }
}

void PointConstraint::OnSnapDistanceChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        snap_dist_ = e_float->get_data();
    }
}

void PointConstraint::OnSnapForceChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // Alternative method for setting the snap distance... we can get a
        // good approximation of the snap distance to use by solving: F = k * x
        // F: Force in Newtons (N).
        // k: Stiffness control coefficient (N/mm).
        // x: Displacement (i.e. snap distance).
        double F = e_float->get_data();
        HDdouble k;
        hdGetDoublev(HD_NOMINAL_MAX_STIFFNESS, &k);
        HDdouble x = F / k;
        // new value will be updated on the next DrawHaptics call
        snap_dist_ = (float)x;
    }
}


void PointConstraint::OnStartEffect() {
    if (!initialized_) {
        shape_id_ = hlGenShapes(1);
        initialized_ = true;
    }
}

void PointConstraint::OnStopEffect() {
}

void PointConstraint::DrawHaptics() {
    hlMaterialf(HL_FRONT, HL_STIFFNESS, stiffness_);
    hlMaterialf(HL_FRONT, HL_DAMPING, damping_);
    hlMaterialf(HL_FRONT, HL_STATIC_FRICTION, static_friction_);
    hlMaterialf(HL_FRONT, HL_DYNAMIC_FRICTION, dynamic_friction_);
    hlTouchModelf(HL_SNAP_DISTANCE, snap_dist_);

    
    hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, shape_id_);
    hlTouchModel(HL_CONSTRAINT);
    glBegin(GL_POINTS);
    glVertex3fv(point_);
    glEnd();
    hlEndShape();
}

void PointConstraint::DrawGraphics() {
    glColor3f(1.0, 1.0, 1.0);
    glPointSize(5.0);
    glBegin(GL_POINTS);
    glVertex3fv(point_);
    glEnd();
}
