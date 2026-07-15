#include "restart_advsettings.h"

#include <windows.h>
#include <tlhelp32.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cwctype>
#include <thread>
#include <vector>

#include <openvr.h>

namespace restart_advsettings {

namespace {

const wchar_t* kTargetProcessName = L"advancedsettings.exe";

bool EqualsIgnoreCaseW(const wchar_t* a, const wchar_t* b) {
    while (*a && *b) {
        if (std::towlower(*a) != std::towlower(*b)) {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == *b;
}

// Terminates every running process named advancedsettings.exe.
// Returns the number of processes it attempted to terminate.
int TerminateAdvancedSettingsProcesses() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    int terminatedCount = 0;

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (EqualsIgnoreCaseW(entry.szExeFile, kTargetProcessName)) {
                HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
                if (process != nullptr) {
                    TerminateProcess(process, 0);
                    CloseHandle(process);
                    ++terminatedCount;
                }
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return terminatedCount;
}

std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// Looks for an installed application key that corresponds to OVR Advanced
// Settings by scanning every key IVRApplications knows about, then falling
// back to the well-known Steam and standalone app keys.
std::string FindAdvancedSettingsAppKey() {
    vr::IVRApplications* apps = vr::VRApplications();
    if (apps == nullptr) {
        return {};
    }

    uint32_t count = apps->GetApplicationCount();
    for (uint32_t i = 0; i < count; ++i) {
        char buffer[vr::k_unMaxApplicationKeyLength] = {};
        vr::EVRApplicationError err = apps->GetApplicationKeyByIndex(i, buffer, sizeof(buffer));
        if (err != vr::VRApplicationError_None) {
            continue;
        }

        std::string key(buffer);
        std::string lowerKey = ToLower(key);
        if (lowerKey.find("advsettings") != std::string::npos ||
            lowerKey.find("1009850") != std::string::npos) {
            return key;
        }
    }

    static const char* kFallbackKeys[] = {
        "steam.overlay.1009850",
        "matzman666.advsettings",
    };

    for (const char* fallback : kFallbackKeys) {
        if (apps->IsApplicationInstalled(fallback)) {
            return fallback;
        }
    }

    return {};
}

} // namespace

Result Restart() {
    Result result;

    int terminated = TerminateAdvancedSettingsProcesses();

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    std::string appKey = FindAdvancedSettingsAppKey();
    if (appKey.empty()) {
        result.success = false;
        result.message = "AdvancedSettings.exe process(es) terminated (" +
            std::to_string(terminated) +
            "), but no OVR Advanced Settings app key was found to relaunch it.";
        return result;
    }

    vr::IVRApplications* apps = vr::VRApplications();
    vr::EVRApplicationError err = apps->LaunchApplication(appKey.c_str());

    if (err == vr::VRApplicationError_None) {
        result.success = true;
        result.message = "Restarted OVR Advanced Settings using app key '" + appKey + "'.";
    } else {
        const char* errName = apps->GetApplicationsErrorNameFromEnum(err);
        result.success = false;
        result.message = "Terminated " + std::to_string(terminated) +
            " process(es), but LaunchApplication('" + appKey + "') failed: " +
            (errName != nullptr ? errName : "unknown error");
    }

    return result;
}

} // namespace restart_advsettings
