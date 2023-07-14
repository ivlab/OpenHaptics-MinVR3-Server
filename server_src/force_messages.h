#ifndef FORCE_SERVER_FORCE_MESSAGES_H
#define FORCE_SERVER_FORCE_MESSAGES_H

#include <string>
#include <vector>

// VREvent -- no additional data
#define PHANTOM_PRIMARY_BTN_DOWN_EVENT_NAME "Phantom/Primary DOWN"
#define PHANTOM_PRIMARY_BTN_UP_EVENT_NAME "Phantom/Primary UP"
// VREventVector3
#define PHANTOM_POSITION_UPDATE_EVENT_NAME "Phantom/Position"
// VREventQuaternion
#define PHANTOM_ROTATION_UPDATE_EVENT_NAME "Phantom/Rotation"


#define MODEL_TO_WORLD_TRANSLATION_UPDATE_EVENT_NAME "Phantom/ModelToWorld/Translation"
#define MODEL_TO_WORLD_ROTATION_UPDATE_EVENT_NAME "Phantom/ModelToWorld/Rotation"
#define MODEL_TO_WORLD_SCALE_UPDATE_EVENT_NAME "Phantom/ModelToWorld/Scale"


#define FORCE_EFFECT_PREFIX "ForceEffect/"


class ForceMessages {
public:

    static std::string get_primary_btn_down_event_name() {
        return std::string(PHANTOM_PRIMARY_BTN_DOWN_EVENT_NAME);
    }

    static std::string get_primary_btn_up_event_name() {
        return std::string(PHANTOM_PRIMARY_BTN_UP_EVENT_NAME);
    }

    static std::string get_position_update_event_name() {
        return std::string(PHANTOM_POSITION_UPDATE_EVENT_NAME);
    }

    static std::string get_rotation_update_event_name() {
        return std::string(PHANTOM_ROTATION_UPDATE_EVENT_NAME);
    }

    static std::string get_model_to_world_translation_event_name() {
        return std::string(MODEL_TO_WORLD_TRANSLATION_UPDATE_EVENT_NAME);
    }

    static std::string get_model_to_world_rotation_event_name() {
        return std::string(MODEL_TO_WORLD_ROTATION_UPDATE_EVENT_NAME);
    }
    
    static std::string get_model_to_world_scale_event_name() {
        return std::string(MODEL_TO_WORLD_SCALE_UPDATE_EVENT_NAME);
    }
    
    static std::string get_force_effect_prefix() {
        return std::string(FORCE_EFFECT_PREFIX);
    }
        
    // splits the name up into its parts using / as the delimeter
    static std::vector<std::string> split_message_name(const std::string &name) {
        std::vector<std::string> tokens;
        const std::string delimiter = "/";
        size_t last = 0;
        size_t next = 0;
        while ((next = name.find(delimiter, last)) != std::string::npos) {
            tokens.push_back(name.substr(last, next-last));
            last = next + 1;
        }
        tokens.push_back(name.substr(last, next-last));
        return tokens;
    }
    
};

#endif
