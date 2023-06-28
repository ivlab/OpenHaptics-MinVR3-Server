#ifndef FORCE_SERVER_FORCE_MESSAGES_H
#define FORCE_SERVER_FORCE_MESSAGES_H

#include <string>
#include <vector>

// VREvent -- no additional data
#define PHANTOM_PRIMARY_DOWN_EVENT_NAME "Phantom/Primary DOWN"
#define PHANTOM_PRIMARY_UP_EVENT_NAME "Phantom/Primary DOWN"
// VREventVector3
#define PHANTOM_POSITION_UPDATE_EVENT_NAME "Phantom/Position"
// VREventQuaternion
#define PHANTOM_ROTATION_UPDATE_EVENT_NAME "Phantom/Position"

// VREventString -- string specifies the name of the effect to start/stop
#define PHANTOM_FORCE_EFFECT_START_EVENT_NAME "ForceEffect/Start"
#define PHANTOM_FORCE_EFFECT_STOP_EVENT_NAME "ForceEffect/Stop"

// VREvent* -- the remainder of the name specifies the name of the effect and parameter, e.g.:
// ForceEffect/Param/AmbientFriction/Gain
// The VREvent can then be cast to whatever type is appropriate for that parameter
#define PHANTOM_FORCE_EFFECT_PARAM_EVENT_NAME_PREFIX "ForceEffect/Param/"


class ForceMessages {
public:

    static std::string get_primary_btn_down_event_name() {
        return std::string(PHANTOM_PRIMARY_DOWN_EVENT_NAME);
    }

    static std::string get_primary_btn_up_event_name() {
        return std::string(PHANTOM_PRIMARY_UP_EVENT_NAME);
    }

    static std::string get_position_update_event_name() {
        return std::string(PHANTOM_POSITION_UPDATE_EVENT_NAME);
    }

    static std::string get_rotation_update_event_name() {
        return std::string(PHANTOM_ROTATION_UPDATE_EVENT_NAME);
    }

    static std::string get_force_effect_start_event_name() {
        return std::string(PHANTOM_FORCE_EFFECT_START_EVENT_NAME);
    }
    
    static std::string get_force_effect_stop_event_name() {
        return std::string(PHANTOM_FORCE_EFFECT_STOP_EVENT_NAME);
    }
    
    static std::string get_force_effect_param_event_name_prefix() {
        return std::string(PHANTOM_FORCE_EFFECT_PARAM_EVENT_NAME_PREFIX);
    }

    static std::string get_force_effect_param_event_name(const std::string &effect_name, const std::string &param_name) {
        return std::string(PHANTOM_FORCE_EFFECT_PARAM_EVENT_NAME_PREFIX) + effect_name + "/" + param_name;
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
