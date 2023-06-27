#ifndef FORCE_SERVER_FORCE_MESSAGES_H
#define FORCE_SERVER_FORCE_MESSAGES_H

#include <string>
#include <vector>

class ForceMessages {
public:
    // Phantom Event Names
    static const std::string k_primary_down = "Phantom/Primary DOWN";
    static const std::string k_primary_up = "Phantom/Primary DOWN";
    static const std::string k_position_update = "Phantom/Position";
    static const std::string k_rotation_update = "Phantom/Rotation";
    
    // Force Effect Event Names
    // 1. base is prepended to every forceffect message
    static const std::string k_effect_base = "ForceEffect";
    // 2. then the name of the effect is appended -- these are defined in the effect classes
    // 3. then one of these commands is appended
    static const std::string k_effect_active = "Active";
    static const std::string k_effect_param = "Param";

    
    static std::string get_active_message_name(const std::string &effect_name) {
        return k_effect_base + "/" + effect_name + "/" + k_effect_active;
    }
    
    static std::string get_param_message_name(const std::string &effect_name) {
        return k_effect_base + "/" + effect_name + "/" + k_effect_param;
    }
    
    // splits the name up into its parts using / as the delimeter
    static std::vector<std::string> split_message_name(const std::string &name) {
        std::vector<std::string> tokens;
        const std::string delimiter = "/";
        size_t last = 0;
        size_t next = 0;
        while ((next = name.find(delimiter, last)) != string::npos) {
            tokens.push_back(name.substr(last, next-last));
            last = next + 1;
        }
        tokens.push_back(name.substr(last, next-last));
        return tokens;
    }
    
};

#endif
