# tcxDmx

DMX-over-serial addon for [TrussC](http://github.com/trussc-org/trussc), targeting the [Enttec DMX USB PRO](https://www.enttec.com/product/protocols/dmx512/dmx-usb-pro-professional-1u-usb-to-dmx512-converter/) and [DMX USB PRO Mk2](https://www.enttec.com/product/protocols/dmx512/dmx-usb-pro-mk2-professional-1u-usb-to-dmx512-converter/).

Ported from [ofxDmx](https://github.com/kylemcdonald/ofxDmx) (originally by Erik Sjodin, later adapted by Marek Bereza, maintained by Kyle McDonald). Built on top of TrussC's built-in `tc::Serial`, so no extra dependency is required.

## Features

- **Send** DMX over the Enttec DMX USB PRO protocol (57600 baud)
- 24–512 channels per universe
- **Mk2 dual-universe** support via `activateMk2()`
- Only sends a packet when channel data changes (or when forced)

## Hardware

- Enttec DMX USB PRO or DMX USB PRO Mk2 (FTDI-based)
- An FTDI Virtual COM Port (VCP) driver — most modern OSes ship one. If yours doesn't, grab it from https://ftdichip.com/drivers/vcp-drivers/

## Platform setup

### macOS

The PRO appears as `/dev/tty.usbserial-XXXXXXXX` (or `/dev/cu.usbserial-XXXXXXXX`). No driver install needed on recent macOS.

### Linux

The PRO appears as `/dev/ttyUSB0` (FTDI). Add your user to the serial group so you can open the port without root:

```
# Debian / Ubuntu
sudo usermod -aG dialout $USER

# Arch / EndeavourOS
sudo usermod -aG uucp $USER
```

Log out and back in for the group change to take effect.

### Windows

Plug in the PRO and let Windows install the FTDI VCP driver (or install it manually from the FTDI link above). The device shows up as `COM3`, `COM4`, etc. — check Device Manager → Ports (COM & LPT).

## Usage

Add the addon to your project's `addons.make`:

```
tcxDmx
```

### Basic send

```cpp
#include "tcxDmx.h"

tcx::Dmx dmx;

void tcApp::setup() {
    dmx.connect(0, 24);          // first detected device, 24 channels
    // dmx.connect("/dev/ttyUSB0", 24);   // or by path
    // dmx.connect("COM3", 24);           // Windows
}

void tcApp::update() {
    dmx.setLevel(1, 255);        // channel 1 → full
    dmx.setLevel(2, 128);
    dmx.update();                // ships a packet only if anything changed
}
```

### Mk2 dual universe

```cpp
dmx.connect(0, 512);
dmx.activateMk2();               // activates universe 2 with the default API key

dmx.setLevel(1,  255, 1);        // universe 1
dmx.setLevel(1,  128, 2);        // universe 2
dmx.update();
```

Mk2 activation is fire-and-forget — the device will not return an error if it's actually a non-Mk2 PRO; the LED on the device blinks **red** when a universe-2 packet is rejected, otherwise universe 2 just lights up.

## Channel numbering

DMX channels are **1-based**. There is no channel 0. Watch the for-loop bounds:

```cpp
const int total = 16;
dmx.connect(0, total);
for (int ch = 1; ch <= total; ch++) {   // <= total, not < total
    dmx.setLevel(ch, value);
}
```

## API

### `tcx::Dmx`

| Method | Description |
| --- | --- |
| `bool connect(int deviceIndex = 0, unsigned int channels = 24)` | Connect to a serial device by index. `channels` is clamped to 24–512. |
| `bool connect(const std::string& devicePath, unsigned int channels = 24)` | Connect by device path (`/dev/ttyUSB0`, `COM3`, etc.). |
| `void disconnect()` | Close the serial port. |
| `bool isConnected() const` | |
| `void activateMk2(uint8_t k0, uint8_t k1, uint8_t k2, uint8_t k3)` | Enable universe 2 on a PRO Mk2. Defaults to the public API key. |
| `void setLevel(unsigned int channel, uint8_t level, unsigned int universe = 1)` | Stage a channel value (1-based, universe 1 or 2). |
| `uint8_t getLevel(unsigned int channel) const` | Read back a staged value (universe 1). |
| `void clear()` | Set all channels in both universes to 0. |
| `void setChannels(unsigned int channels)` | Resize. Clamped to 24–512. |
| `void update(bool force = false)` | Ship a packet if anything changed (or always, if `force`). |

## Examples

- `example-dmx-basic` — three channels driven by sin/cos auto-cycle
- `example-dmx-controller` — tcxImGui sliders for RGB modules + save/load preset

## Tested

- [x] Linux (EndeavourOS / Arch)
- [ ] Linux (Ubuntu / Debian)
- [ ] Windows
- [ ] macOS

## Credits

- [ofxDmx](https://github.com/kylemcdonald/ofxDmx) — the openFrameworks addon this is ported from
- Erik Sjodin (original DmxPro), Marek Bereza, Kyle McDonald, and ofxDmx contributors

## License

MIT License. See [LICENSE](LICENSE).
