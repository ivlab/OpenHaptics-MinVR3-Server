
#include "ambient_viscous.h"

#include "force_messages.h"


AmbientViscous::AmbientViscous(EventMgr* event_mgr) : gain_(0.8), magnitude_cap_(1.0) {
    std::string gain_event_name = ForceMessages::get_force_effect_param_event_name("AmbientViscous", "Gain");
    event_mgr->AddListener(gain_event_name, this, &AmbientViscous::OnGainChange);

    std::string mag_event_name = ForceMessages::get_force_effect_param_event_name("AmbientViscous", "MagnitudeCap");
    event_mgr->AddListener(mag_event_name, this, &AmbientViscous::OnMagnitudeCapChange);
}

AmbientViscous::~AmbientViscous() {
    
}

void AmbientViscous::OnGainChange(VREvent* e) {
    VREventFloat* e_gain = dynamic_cast<VREventFloat*>(e);
    if (e_gain != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        gain_ = e_gain->get_data();
    }
}

void AmbientViscous::OnMagnitudeCapChange(VREvent* e) {
    VREventFloat* e_mag = dynamic_cast<VREventFloat*>(e);
    if (e_mag != NULL) { // should always pass
        // new value will be updated on the next DrawHaptics call
        magnitude_cap_ = e_mag->get_data();
    }
}


void AmbientViscous::Init() {
    effect_id_ = hlGenEffects(1);
}

void AmbientViscous::OnStartEffect() {
    hlEffectd(HL_EFFECT_PROPERTY_GAIN, gain_);
    hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, magnitude_cap_);
    hlStartEffect(HL_EFFECT_FRICTION, effect_id_);
}

void AmbientViscous::OnStopEffect() {
    hlStopEffect(effect_id_);
}

void AmbientViscous::DrawHaptics() {
    hlEffectd(HL_EFFECT_PROPERTY_GAIN, gain_);
    hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, magnitude_cap_);
    hlUpdateEffect(effect_id_);
}

void AmbientViscous::DrawGraphics() {
    
}

