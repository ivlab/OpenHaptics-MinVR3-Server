
#include "line_constraint.h"

#include <iostream>
#include "force_messages.h"
#include "phantom.h"


LineConstraint::LineConstraint(EventMgr* event_mgr) : active_(false) {
    stiffness_ = 0.2f;        // 0..1
    damping_ = 0.2f;          // 0..1
    static_friction_ = 0.2f;  // 0..1
    dynamic_friction_ = 0.2f; // 0..1
    snap_dist_ = 10.0f;       // dist in mm
    
    std::string event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Start";
    event_mgr->AddListener(event_name, this, &LineConstraint::OnStartEffect);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Stop";
    event_mgr->AddListener(event_name, this, &LineConstraint::OnStopEffect);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/BeginLines";
    event_mgr->AddListener(event_name, this, &LineConstraint::OnBeginLines);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/AddVertex";
    event_mgr->AddListener(event_name, this, &LineConstraint::OnAddVertex);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/EndLines";
    event_mgr->AddListener(event_name, this, &LineConstraint::OnEndLines);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetStiffness";
    event_mgr->AddListener(event_name, this, &LineConstraint::OnStiffnessChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetDamping";
    event_mgr->AddListener(event_name, this, &LineConstraint::OnDampingChange);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetStaticFriction";
    event_mgr->AddListener(event_name, this, &LineConstraint::OnStaticFrictionChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetDynamicFriction";
    event_mgr->AddListener(event_name, this, &LineConstraint::OnDynamicFrictionChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetSnapDistance";
    event_mgr->AddListener(event_name, this, &LineConstraint::OnSnapDistanceChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetSnapForce";
    event_mgr->AddListener(event_name, this, &LineConstraint::OnSnapForceChange);
}

LineConstraint::~LineConstraint() {
}



void LineConstraint::OnStartEffect(VREvent* e) {
    active_ = true;
}

void LineConstraint::OnStopEffect(VREvent* e) {
    active_ = false;
}

void LineConstraint::Reset() {
    verts_.clear();
    verts_tmp_buffer_.clear();
    active_ = false;
}


void LineConstraint::OnBeginLines(VREvent* e) {
    // add to a tmp buffer and only "commit" the vertices to the points list used in the
    // haptic draw method after receiving ALL of the points.
    verts_tmp_buffer_.clear();
}

void LineConstraint::OnAddVertex(VREvent* e) {
    VREventVector3* e_p = dynamic_cast<VREventVector3*>(e);
    if (e_p != NULL) { // should always pass
        hduVector3Df v(e_p->x(), e_p->y(), e_p->z());
        verts_tmp_buffer_.push_back(v);
    }
}

void LineConstraint::OnEndLines(VREvent* e) {
    verts_ = verts_tmp_buffer_;
}

void LineConstraint::OnStiffnessChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        stiffness_ = e_float->get_data();
    }
}

void LineConstraint::OnDampingChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        damping_ = e_float->get_data();
    }
}

void LineConstraint::OnStaticFrictionChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        static_friction_ = e_float->get_data();
    }
}

void LineConstraint::OnDynamicFrictionChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        dynamic_friction_ = e_float->get_data();
    }
}

void LineConstraint::OnSnapDistanceChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        snap_dist_ = e_float->get_data();
    }
}

void LineConstraint::OnSnapForceChange(VREvent* e) {
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


void LineConstraint::DrawHaptics() {
    if ((active_) && (!verts_.empty())) {
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
        glBegin(GL_LINES);
        for (int i=0; i<verts_.size(); i++) {
            glVertex3f(verts_[i][0], verts_[i][1], verts_[i][2]);
        }
        glEnd();
        hlEndShape();
        Phantom::CheckHapticError();
    }
}

void LineConstraint::DrawGraphics() {
    if ((active_) && (!verts_.empty())) {
        glDisable(GL_LIGHTING);
        glColor3f(0.3, 0.3, 1.0);
        glBegin(GL_LINES);
        for (int i=0; i<verts_.size(); i++) {
            glVertex3f(verts_[i][0], verts_[i][1], verts_[i][2]);
        }
        glEnd();
        glEnable(GL_LIGHTING);
    }
}
