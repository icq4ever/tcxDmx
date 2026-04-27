#include "TrussC.h"
#include "tcApp.h"

int main() {
    WindowSettings settings;
    settings.title   = "DMX Controller Example";
    settings.width   = 720;
    settings.height  = 520;
    settings.highDpi = false;
    return TC_RUN_APP(tcApp, settings);
}
