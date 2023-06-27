
#include "ambient_friction.h"

#include "force_messages.h"


AmbientFriction::AmbientFriction(EventMgr* event_mgr) : gain_(0.15), magnitude_cap_(0.075) {
    event_mgr->AddListener(this, &AmbientFriction::OnParamChange);
    event_mgr->AddListenerByName(ForceMessages::get_param_message_name(Name()), this, &AmbientFriction::OnParamChange);
}

AmbientFriction::~AmbientFriction() {
    
}

void AmbientFriction::OnParamChange(VREvent* e) {
    // e.g. VREventFloat("ForceEffect/AmbientFriction/Param/Gain", 0.2)
    std::vector<std::string> event_name_parts = ForceMessages::split_message_name(e->get_name());
    if (event_name_parts.size() == 4) { // should always pass
        std::string param = event_name_parts[3];
        if (param == "Gain") {
            VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
            if (e_float != NULL) {
                gain_ = e_float->get_data();
            }
        }
        else if (param == "MagnitudeCap") {
            VREventFloat* e_float = dynamic_cast<VREventFloat*>(e);
            if (e_float != NULL) {
                magnitude_cap_ = e_float->get_data();
            }
        }
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
