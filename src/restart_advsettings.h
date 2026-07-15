#pragma once

#include <string>

namespace restart_advsettings {

struct Result {
    bool success = false;
    std::string message;
};

// Terminates any running AdvancedSettings.exe process(es) and relaunches
// OVR Advanced Settings through IVRApplications::LaunchApplication.
Result Restart();

} // namespace restart_advsettings
