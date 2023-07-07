
#include "surface_constraint.h"

#include <iostream>
#include "force_messages.h"
#include "phantom.h"


SurfaceConstraint::SurfaceConstraint(EventMgr* event_mgr) : active_(false) {
    stiffness_ = 0.2f;        // 0..1
    damping_ = 0.2f;          // 0..1
    static_friction_ = 0.2f;  // 0..1
    dynamic_friction_ = 0.2f; // 0..1
    snap_dist_ = 10.0f;       // dist in mm
    
    std::string event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Start";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnStartEffect);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Stop";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnStopEffect);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/BeginGeometry";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnBeginGeometry);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/AddVertex";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnAddVertex);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/AddIndices";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnAddIndices);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/EndGeometry";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnEndGeometry);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetStiffness";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnStiffnessChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetDamping";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnDampingChange);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetStaticFriction";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnStaticFrictionChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetDynamicFriction";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnDynamicFrictionChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetSnapDistance";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnSnapDistanceChange);
    
    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetSnapForce";
    event_mgr->AddListener(event_name, this, &SurfaceConstraint::OnSnapForceChange);
}

SurfaceConstraint::~SurfaceConstraint() {
}



void SurfaceConstraint::OnStartEffect(VREvent* e) {
    active_ = true;
}

void SurfaceConstraint::OnStopEffect(VREvent* e) {
    active_ = false;
}

void SurfaceConstraint::OnBeginGeometry(VREvent* e) {
    // add to a tmp buffer and only "commit" the vertices to the points list used in the
    // haptic draw method after receiving ALL of the points.
    verts_tmp_buffer_.clear();
    indices_tmp_buffer_.clear();
}

void SurfaceConstraint::OnAddVertex(VREvent* e) {
    VREventVector3* e_p = dynamic_cast<VREventVector3*>(e);
    if (e_p != NULL) { // should always pass
        hduVector3Df v(e_p->x(), e_p->y(), e_p->z());
        verts_tmp_buffer_.push_back(v);
    }
}

void SurfaceConstraint::OnAddIndices(VREvent* e) {
    VREventVector3* e_p = dynamic_cast<VREventVector3*>(e);
    if (e_p != NULL) { // should always pass
        indices_tmp_buffer_.push_back((int)e_p->x());
        indices_tmp_buffer_.push_back((int)e_p->y());
        indices_tmp_buffer_.push_back((int)e_p->z());
    }
}


void SurfaceConstraint::OnEndGeometry(VREvent* e) {
    verts_ = verts_tmp_buffer_;
    indices_ = indices_tmp_buffer_;
}

void SurfaceConstraint::OnStiffnessChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        stiffness_ = e_float->get_data();
    }
}

void SurfaceConstraint::OnDampingChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        damping_ = e_float->get_data();
    }
}

void SurfaceConstraint::OnStaticFrictionChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        static_friction_ = e_float->get_data();
    }
}

void SurfaceConstraint::OnDynamicFrictionChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        dynamic_friction_ = e_float->get_data();
    }
}

void SurfaceConstraint::OnSnapDistanceChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        snap_dist_ = e_float->get_data();
    }
}

void SurfaceConstraint::OnSnapForceChange(VREvent* e) {
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


void SurfaceConstraint::DrawHaptics() {
    if ((active_) && (!verts_.empty()) && (!indices_.empty())) {
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
        glBegin(GL_TRIANGLES);
        for (int i=0; i<indices_.size(); i++) {
            glVertex3f(verts_[indices_[i]][0], verts_[indices_[i]][1], verts_[indices_[i]][2]);
        }
        glEnd();
        hlEndShape();
        Phantom::CheckHapticError();
    }
}

void SurfaceConstraint::DrawGraphics() {
    if ((active_) && (!verts_.empty())) {
        glColor3f(1.0, 1.0, 1.0);
        glPointSize(5.0);
        glBegin(GL_TRIANGLES);
        for (int i=0; i<indices_.size(); i++) {
            glVertex3f(verts_[indices_[i]][0], verts_[indices_[i]][1], verts_[indices_[i]][2]);
        }
        glEnd();
    }
}
