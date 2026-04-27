#pragma once

#include <TrussC.h>
#include <tcxDmx.h>

using namespace tc;

class tcApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;

private:
    tcx::Dmx dmx;
    bool     autoCycle = true;
    uint8_t  chan1 = 120, chan2 = 120, chan3 = 120;

    // Optional: pin to a specific port. Empty → first detected device.
    // e.g. "/dev/ttyUSB0", "COM3", "/dev/tty.usbserial-EN143965"
    std::string portPath = "";
};
