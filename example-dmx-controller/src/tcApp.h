#pragma once

#include <TrussC.h>
#include <tcxDmx.h>
#include <tcxImGui.h>

#include <array>
#include <string>
#include <vector>

using namespace tc;
using namespace tcx;

// 6 RGB modules × 4 channels per module (R,G,B,unused) = 24 channels.
// Matches the channel layout of ofxDmx's example-controller.
class tcApp : public App {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void cleanup() override;

private:
    static constexpr int kModules           = 6;
    static constexpr int kChannelsPerModule = 4;
    static constexpr int kTotalChannels     = kModules * kChannelsPerModule;

    Dmx                              dmx;
    std::vector<SerialDeviceInfo>    devices;
    int                              selectedDevice = -1;
    bool                             enableMk2      = false;

    // Per-module RGB sliders (0..1). Index 0 is unused so module N maps to [N].
    std::array<float, kModules + 1> red{};
    std::array<float, kModules + 1> green{};
    std::array<float, kModules + 1> blue{};

    void rescan();
    void connectSelected();
};
