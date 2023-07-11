
#include "surface_contact.h"

#include <iostream>
#include "force_messages.h"
#include "phantom.h"


SurfaceContact::SurfaceContact(EventMgr* event_mgr) : active_(false) {
    stiffness_ = 0.2f;        // 0..1
    damping_ = 0.2f;          // 0..1
    static_friction_ = 0.2f;  // 0..1
    dynamic_friction_ = 0.2f; // 0..1

    std::string event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Start";
    event_mgr->AddListener(event_name, this, &SurfaceContact::OnStartEffect);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/Stop";
    event_mgr->AddListener(event_name, this, &SurfaceContact::OnStopEffect);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/BeginGeometry";
    event_mgr->AddListener(event_name, this, &SurfaceContact::OnBeginGeometry);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/AddVertex";
    event_mgr->AddListener(event_name, this, &SurfaceContact::OnAddVertex);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/AddIndices";
    event_mgr->AddListener(event_name, this, &SurfaceContact::OnAddIndices);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/EndGeometry";
    event_mgr->AddListener(event_name, this, &SurfaceContact::OnEndGeometry);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetStiffness";
    event_mgr->AddListener(event_name, this, &SurfaceContact::OnStiffnessChange);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetDamping";
    event_mgr->AddListener(event_name, this, &SurfaceContact::OnDampingChange);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetStaticFriction";
    event_mgr->AddListener(event_name, this, &SurfaceContact::OnStaticFrictionChange);

    event_name = ForceMessages::get_force_effect_prefix() + Name() + "/SetDynamicFriction";
    event_mgr->AddListener(event_name, this, &SurfaceContact::OnDynamicFrictionChange);
}

SurfaceContact::~SurfaceContact() {
}



void SurfaceContact::OnStartEffect(VREvent* e) {
    active_ = true;
}

void SurfaceContact::OnStopEffect(VREvent* e) {
    active_ = false;
}

void SurfaceContact::Reset() {
    verts_.clear();
    verts_tmp_buffer_.clear();
    indices_.clear();
    indices_tmp_buffer_.clear();
    active_ = false;
}


void SurfaceContact::OnBeginGeometry(VREvent* e) {
    // add to a tmp buffer and only "commit" the vertices to the points list used in the
    // haptic draw method after receiving ALL of the points.
    verts_tmp_buffer_.clear();
    indices_tmp_buffer_.clear();
}

void SurfaceContact::OnAddVertex(VREvent* e) {
    VREventVector3* e_p = dynamic_cast<VREventVector3*>(e);
    if (e_p != NULL) { // should always pass
        hduVector3Df v(e_p->x(), e_p->y(), e_p->z());
        verts_tmp_buffer_.push_back(v);
    }
}

void SurfaceContact::OnAddIndices(VREvent* e) {
    VREventVector3* e_p = dynamic_cast<VREventVector3*>(e);
    if (e_p != NULL) { // should always pass
        indices_tmp_buffer_.push_back((int)e_p->x());
        indices_tmp_buffer_.push_back((int)e_p->y());
        indices_tmp_buffer_.push_back((int)e_p->z());
    }
}


void SurfaceContact::OnEndGeometry(VREvent* e) {
    verts_ = verts_tmp_buffer_;
    indices_ = indices_tmp_buffer_;
}

void SurfaceContact::OnStiffnessChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        stiffness_ = e_float->get_data();
    }
}

void SurfaceContact::OnDampingChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        damping_ = e_float->get_data();
    }
}

void SurfaceContact::OnStaticFrictionChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        static_friction_ = e_float->get_data();
    }
}

void SurfaceContact::OnDynamicFrictionChange(VREvent* e) {
    VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
    if (e_float != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        dynamic_friction_ = e_float->get_data();
    }
}

void SurfaceContact::DrawHaptics() {
    if ((active_) && (!verts_.empty()) && (!indices_.empty())) {
        if (!hlIsShape(shape_id_)) {
            shape_id_ = hlGenShapes(1);
            Phantom::CheckHapticError();
        }

        hlTouchableFace(HL_FRONT);

        hlMaterialf(HL_FRONT, HL_STIFFNESS, stiffness_);
        hlMaterialf(HL_FRONT, HL_DAMPING, damping_);
        hlMaterialf(HL_FRONT, HL_STATIC_FRICTION, static_friction_);
        hlMaterialf(HL_FRONT, HL_DYNAMIC_FRICTION, dynamic_friction_);

        hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, shape_id_);
        hlTouchModel(HL_CONTACT);
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < indices_.size(); i++) {
            glVertex3f(verts_[indices_[i]][0], verts_[indices_[i]][1], verts_[indices_[i]][2]);
        }
        glEnd();
        hlEndShape();
        Phantom::CheckHapticError();
    }
}

void SurfaceContact::DrawGraphics() {
    if ((active_) && (!verts_.empty())) {
        glColor3f(1.0, 1.0, 0.3);
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < indices_.size(); i++) {
            glVertex3f(verts_[indices_[i]][0], verts_[indices_[i]][1], verts_[indices_[i]][2]);
        }
        glEnd();
    }
}
