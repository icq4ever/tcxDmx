#include "TrussC.h"
#include "tcApp.h"

int main() {
    WindowSettings settings;
    settings.title   = "DMX Basic Example";
    settings.width   = 600;
    settings.height  = 300;
    settings.highDpi = false;
    return TC_RUN_APP(tcApp, settings);
}
