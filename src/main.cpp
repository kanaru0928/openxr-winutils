#include <windows.h>

#include "overlay_app.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    OverlayApp app;
    if (!app.Init()) {
        app.Shutdown();
        return 1;
    }

    while (app.Tick()) {
        // All work happens inside Tick(); this loop just keeps the
        // application alive until SteamVR requests a quit.
    }

    app.Shutdown();
    return 0;
}
