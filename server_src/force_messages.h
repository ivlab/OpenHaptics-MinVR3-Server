#ifndef FORCE_SERVER_FORCE_MESSAGES_H
#define FORCE_SERVER_FORCE_MESSAGES_H

#include <string>
#include <vector>

#define PHANTOM_PRIMARY_DOWN_EVENT_NAME "Phantom/Primary DOWN"
#define PHANTOM_PRIMARY_UP_EVENT_NAME "Phantom/Primary DOWN"
#define PHANTOM_POSITION_UPDATE_EVENT_NAME "Phantom/Position"
#define PHANTOM_ROTATION_UPDATE_EVENT_NAME "Phantom/Position"

#define PHANTOM_FORCE_EFFECT_EVENT_BASE_NAME_PART "ForceEffect"
#define PHANTOM_FORCE_EFFECT_EVENT_ACTIVE_NAME_PART "Active"
#define PHANTOM_FORCE_EFFECT_EVENT_PARAM_NAME_PART "Param"


class ForceMessages {
public:

    static std::string get_primary_btn_down_message_name() {
        return std::string(PHANTOM_PRIMARY_DOWN_EVENT_NAME);
    }

    static std::string get_primary_btn_up_message_name() {
        return std::string(PHANTOM_PRIMARY_UP_EVENT_NAME);
    }

    static std::string get_position_update_message_name() {
        return std::string(PHANTOM_POSITION_UPDATE_EVENT_NAME);
    }

    static std::string get_rotation_update_message_name() {
        return std::string(PHANTOM_ROTATION_UPDATE_EVENT_NAME);
    }

    static std::string get_active_message_name(const std::string &effect_name) {
        return std::string(PHANTOM_FORCE_EFFECT_EVENT_BASE_NAME_PART "/") + effect_name + std::string("/" PHANTOM_FORCE_EFFECT_EVENT_ACTIVE_NAME_PART);
    }
    
    static std::string get_param_message_name(const std::string &effect_name) {
        return std::string(PHANTOM_FORCE_EFFECT_EVENT_BASE_NAME_PART "/") + effect_name + std::string("/" PHANTOM_FORCE_EFFECT_EVENT_ACTIVE_NAME_PART);
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
