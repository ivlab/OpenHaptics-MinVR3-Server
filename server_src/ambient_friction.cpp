
#include "ambient_friction.h"

#include "force_messages.h"


AmbientFriction::AmbientFriction(EventMgr* event_mgr) : gain_(0.15), magnitude_cap_(0.075) {
    std::string gain_event_name = ForceMessages::get_force_effect_param_event_name(Name(), "Gain");
    event_mgr->AddListener(gain_event_name, this, &AmbientFriction::OnGainChange);

    std::string mag_event_name = ForceMessages::get_force_effect_param_event_name(Name(), "MagnitudeCap");
    event_mgr->AddListener(mag_event_name, this, &AmbientFriction::OnMagnitudeCapChange);
}

AmbientFriction::~AmbientFriction() {
    
}

void AmbientFriction::OnGainChange(VREvent* e) {
    VREventFloat* e_gain = dynamic_cast<VREventFloat*>(e);
    if (e_gain != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        gain_ = e_gain->get_data();
    }
}

void AmbientFriction::OnMagnitudeCapChange(VREvent* e) {
    VREventFloat* e_mag = dynamic_cast<VREventFloat*>(e);
    if (e_mag != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        magnitude_cap_ = e_mag->get_data();
    }
}


void AmbientFriction::Init() {
    effect_id_ = hlGenEffects(1);
}

void AmbientFriction::OnStartEffect() {
    hlEffectd(HL_EFFECT_PROPERTY_GAIN, gain_);
    hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, magnitude_cap_);
    hlStartEffect(HL_EFFECT_FRICTION, effect_id_);
}

void AmbientFriction::OnStopEffect() {
    hlStopEffect(effect_id_);
}

void AmbientFriction::DrawHaptics() {
    hlEffectd(HL_EFFECT_PROPERTY_GAIN, gain_);
    hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, magnitude_cap_);
    hlUpdateEffect(effect_id_);
}

void AmbientFriction::DrawGraphics() {
    
}
