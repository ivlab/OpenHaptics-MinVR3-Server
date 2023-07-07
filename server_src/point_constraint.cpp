
#include "point_constraint.h"

#include <iostream>
#include "force_messages.h"
#include "phantom.h"


PointConstraint::PointConstraint(EventMgr* event_mgr) : active_(false) {
    stiffness_ = 0.2f;        // 0..1
    damping_ = 0.2f;          // 0..1
    static_friction_ = 0.2f;  // 0..1
    dynamic_friction_ = 0.2f; // 0..1
    snap_dist_ = 10.0f;       // dist in mm
    
    std::string event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Start";
    event_mgr->AddListener(event_name, this, &PointConstraint::OnStartEffect);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Stop";
    event_mgr->AddListener(event_name, this, &PointConstraint::OnStopEffect);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/BeginPoints";
    event_mgr->AddListener(event_name, this, &PointConstraint::OnBeginPoints);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/AddVertex";
    event_mgr->AddListener(event_name, this, &PointConstraint::OnAddVertex);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/EndPoints";
    event_mgr->AddListener(event_name, this, &PointConstraint::OnEndPoints);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetStiffness";
    event_mgr->AddListener(event_name, this, &PointConstraint::OnStiffnessChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetDamping";
    event_mgr->AddListener(event_name, this, &PointConstraint::OnDampingChange);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetStaticFriction";
    event_mgr->AddListener(event_name, this, &PointConstraint::OnStaticFrictionChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetDynamicFriction";
    event_mgr->AddListener(event_name, this, &PointConstraint::OnDynamicFrictionChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetSnapDistance";
    event_mgr->AddListener(event_name, this, &PointConstraint::OnSnapDistanceChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetSnapForce";
    event_mgr->AddListener(event_name, this, &PointConstraint::OnSnapForceChange);
}

PointConstraint::~PointConstraint() {
}



void PointConstraint::OnStartEffect(VREvent* e) {
    active_ = true;
}

void PointConstraint::OnStopEffect(VREvent* e) {
    active_ = false;
}

void PointConstraint::Reset() {
    points_.clear();
    points_tmp_buffer_.clear();
    active_ = false;
}


void PointConstraint::OnBeginPoints(VREvent* e) {
    // add to a tmp buffer and only "commit" the vertices to the points list used in the
    // haptic draw method after receiving ALL of the points.
    points_tmp_buffer_.clear();
}

void PointConstraint::OnAddVertex(VREvent* e) {
    VREventVector3* e_p = dynamic_cast<VREventVector3*>(e);
    if (e_p != NULL) { // should always pass
        hduVector3Df v(e_p->x(), e_p->y(), e_p->z());
        points_tmp_buffer_.push_back(v);
    }
}

void PointConstraint::OnEndPoints(VREvent* e) {
    points_ = points_tmp_buffer_;
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


void PointConstraint::DrawHaptics() {
    if ((active_) && (!points_.empty())) {
        if (!hlIsShape(shape_id_)) {
            shape_id_ = hlGenShapes(1);
            Phantom::CheckHapticError();
        }

        hlMaterialf(HL_FRONT, HL_STIFFNESS, stiffness_);
        hlMaterialf(HL_FRONT, HL_DAMPING, damping_);
        hlMaterialf(HL_FRONT, HL_STATIC_FRICTION, static_friction_);
        hlMaterialf(HL_FRONT, HL_DYNAMIC_FRICTION, dynamic_friction_);
        hlTouchModelf(HL_SNAP_DISTANCE, snap_dist_);

        hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, shape_id_);
        hlTouchModel(HL_CONSTRAINT);
        glBegin(GL_POINTS);
        for (int i=0; i<points_.size(); i++) {
            glVertex3f(points_[i][0], points_[i][1], points_[i][2]);
        }
        glEnd();
        hlEndShape();
        Phantom::CheckHapticError();
    }
}

void PointConstraint::DrawGraphics() {
    if ((active_) && (!points_.empty())) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0, 1.0, 1.0);
        glPointSize(5.0);
        glBegin(GL_POINTS);
        for (int i=0; i<points_.size(); i++) {
            glVertex3f(points_[i][0], points_[i][1], points_[i][2]);
        }
        glEnd();
        glEnable(GL_LIGHTING);
    }
}
