#include "tcApp.h"

void tcApp::setup() {
    setFps(VSYNC);
    imguiSetup();
    rescan();
}

void tcApp::cleanup() {
    if (dmx.isConnected()) {
        dmx.clear();
        dmx.update(true); // black on shutdown
    }
    imguiShutdown();
}

void tcApp::rescan() {
    devices = dmx.isConnected() ? std::vector<SerialDeviceInfo>{}
                                : Serial::listDevices();
    if (selectedDevice < 0 && !devices.empty()) {
        selectedDevice = 0;
    }
}

void tcApp::connectSelected() {
    if (selectedDevice < 0 || selectedDevice >= (int)devices.size()) return;
    dmx.connect(devices[selectedDevice].devicePath, kTotalChannels);
    if (dmx.isConnected() && enableMk2) {
        dmx.activateMk2();
    }
    dmx.update(true); // black on startup
}

void tcApp::update() {
    if (!dmx.isConnected()) return;

    int channel = 1;
    for (int m = 1; m <= kModules; ++m) {
        dmx.setLevel(channel++, static_cast<uint8_t>(red[m]   * 255.0f));
        dmx.setLevel(channel++, static_cast<uint8_t>(green[m] * 255.0f));
        dmx.setLevel(channel++, static_cast<uint8_t>(blue[m]  * 255.0f));
        ++channel; // skip the 4th (unused) channel of each module
    }
    dmx.update();
}

void tcApp::draw() {
    clear(0.08f, 0.08f, 0.10f);

    // Module color swatches on the left.
    const float swatch = 36.0f;
    const float pad    = 8.0f;
    for (int m = 1; m <= kModules; ++m) {
        const float y = 16 + (m - 1) * (swatch + pad);
        setColor(red[m], green[m], blue[m]);
        drawRect(16, y, swatch, swatch);
        setColor(1.0f);
        drawBitmapString("module " + std::to_string(m), 16 + swatch + 12, y + 22);
    }

    // ImGui controls on the right.
    imguiBegin();

    ImGui::SetNextWindowPos(ImVec2(220, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(getWindowWidth() - 240.0f, getWindowHeight() - 20.0f),
                             ImGuiCond_FirstUseEver);
    ImGui::Begin("tcxDmx controller");

    // Device selection
    ImGui::Text("Connection");
    if (ImGui::Button("Rescan")) {
        rescan();
    }
    ImGui::SameLine();
    const std::string preview = (selectedDevice >= 0 && selectedDevice < (int)devices.size())
                              ? devices[selectedDevice].devicePath
                              : std::string("(none)");
    if (ImGui::BeginCombo("port", preview.c_str())) {
        for (int i = 0; i < (int)devices.size(); ++i) {
            const bool isSelected = (selectedDevice == i);
            if (ImGui::Selectable(devices[i].devicePath.c_str(), isSelected)) {
                selectedDevice = i;
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Checkbox("activate Mk2 universe 2", &enableMk2);
    if (!dmx.isConnected()) {
        if (ImGui::Button("Connect")) connectSelected();
    } else {
        if (ImGui::Button("Disconnect")) dmx.disconnect();
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "connected");
    }

    ImGui::Separator();
    ImGui::Text("RGB modules (channels 1..%d)", kTotalChannels);

    for (int m = 1; m <= kModules; ++m) {
        ImGui::PushID(m);
        ImGui::Text("module %d  (ch %d-%d)", m,
                    (m - 1) * kChannelsPerModule + 1,
                    (m - 1) * kChannelsPerModule + 3);
        ImGui::SliderFloat("red",   &red[m],   0.0f, 1.0f);
        ImGui::SliderFloat("green", &green[m], 0.0f, 1.0f);
        ImGui::SliderFloat("blue",  &blue[m],  0.0f, 1.0f);
        ImGui::PopID();
    }

    if (ImGui::Button("All off")) {
        for (int m = 1; m <= kModules; ++m) {
            red[m] = green[m] = blue[m] = 0.0f;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("All full")) {
        for (int m = 1; m <= kModules; ++m) {
            red[m] = green[m] = blue[m] = 1.0f;
        }
    }

    ImGui::End();
    imguiEnd();
}
