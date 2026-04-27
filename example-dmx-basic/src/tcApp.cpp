#include "tcApp.h"

#include <cmath>

void tcApp::setup() {
    setFps(VSYNC);

    if (!portPath.empty()) {
        dmx.connect(portPath, 24);
    } else {
        dmx.connect(0, 24);
    }
}

void tcApp::update() {
    if (autoCycle) {
        const float t = static_cast<float>(getElapsedTime());
        chan1 = static_cast<uint8_t>(127 + 127 * std::sin(2.0f * t));
        chan2 = static_cast<uint8_t>(127 + 127 * std::sin(-2.0f * t));
        chan3 = static_cast<uint8_t>(127 + 127 * std::sin(1.5f * t));
    }

    dmx.setLevel(1, chan1);
    dmx.setLevel(2, chan2);
    dmx.setLevel(3, chan3);
    dmx.update();
}

void tcApp::draw() {
    clear(0.0f);

    const float w = getWindowWidth() / 3.0f;
    const float h = static_cast<float>(getWindowHeight());

    setColor(chan1 / 255.0f);
    drawRect(0,     0, w, h);

    setColor(chan2 / 255.0f);
    drawRect(w,     0, w, h);

    setColor(chan3 / 255.0f);
    drawRect(2 * w, 0, w, h);

    setColor(1.0f);
    drawBitmapString(std::string("DMX: ") + (dmx.isConnected() ? "connected" : "no device"),
                     12, 24);
    drawBitmapString(std::string("auto-cycle: ") + (autoCycle ? "on" : "off") + "  (SPACE)",
                     12, 44);
    drawBitmapString("ch1=" + std::to_string(chan1) +
                     "  ch2=" + std::to_string(chan2) +
                     "  ch3=" + std::to_string(chan3), 12, 64);
}

void tcApp::keyPressed(int key) {
    if (key == KEY_SPACE) {
        autoCycle = !autoCycle;
    }
}
