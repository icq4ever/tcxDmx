#include "tcxDmx.h"

#include <algorithm>

// Enttec DMX USB PRO protocol
// Reference: Enttec USB Pro API spec / ofxDmx (https://github.com/icq4ever/ofxDmx)
namespace {
constexpr uint8_t  DMX_PRO_START_MSG       = 0x7E;
constexpr uint8_t  DMX_PRO_END_MSG         = 0xE7;
constexpr uint8_t  DMX_PRO_SEND_PACKET     = 6;     // periodic DMX packet, universe 1
constexpr uint8_t  DMX_PRO_SEND_PACKET2    = 0xA9;  // Mk2 universe 2
constexpr uint8_t  ENTTEC_PRO_ENABLE_API2  = 0x0D;
constexpr uint8_t  ENTTEC_PRO_PORT_ASS_REQ = 0xCB;
constexpr uint8_t  DMX_START_CODE          = 0;
constexpr unsigned DMX_PRO_HEADER_SIZE     = 4;
constexpr unsigned DMX_START_CODE_SIZE     = 1;
constexpr unsigned DMX_PRO_END_SIZE        = 1;
constexpr int      DMX_PRO_BAUD            = 57600;
constexpr unsigned DMX_MIN_CHANNELS        = 24;
constexpr unsigned DMX_MAX_CHANNELS        = 512;
} // namespace

namespace tcx {

Dmx::Dmx() = default;

Dmx::~Dmx() {
    disconnect();
}

bool Dmx::connect(int deviceIndex, unsigned int channels) {
    serial_.listDevices();
    connected_ = serial_.setup(deviceIndex, DMX_PRO_BAUD);
    setChannels(channels);
    return connected_;
}

bool Dmx::connect(const std::string& devicePath, unsigned int channels) {
    serial_.listDevices();
    connected_ = serial_.setup(devicePath, DMX_PRO_BAUD);
    setChannels(channels);
    return connected_;
}

void Dmx::disconnect() {
    serial_.close();
    connected_ = false;
}

void Dmx::setChannels(unsigned int channels) {
    const auto n = std::clamp(channels, DMX_MIN_CHANNELS, DMX_MAX_CHANNELS);
    levels_.assign(n, 0);
    levels2_.assign(n, 0);
}

void Dmx::activateMk2(uint8_t key0, uint8_t key1, uint8_t key2, uint8_t key3) {
    // Step 1: send the API key.
    {
        constexpr unsigned dataSize   = 4;
        constexpr unsigned packetSize = DMX_PRO_HEADER_SIZE + dataSize + DMX_PRO_END_SIZE;
        std::vector<uint8_t> packet(packetSize);

        packet[0] = DMX_PRO_START_MSG;
        packet[1] = ENTTEC_PRO_ENABLE_API2;
        packet[2] = static_cast<uint8_t>(dataSize & 0xFF);
        packet[3] = static_cast<uint8_t>((dataSize >> 8) & 0xFF);

        // API key, LSB first
        packet[4] = key3;
        packet[5] = key2;
        packet[6] = key1;
        packet[7] = key0;

        packet[packetSize - 1] = DMX_PRO_END_MSG;

        serial_.writeBytes(packet.data(), static_cast<int>(packetSize));
    }
    tc::sleepMillis(200);
    tc::logNotice("tcxDmx") << "MK2: activated with API key";

    // Step 2: enable both ports.
    {
        constexpr unsigned dataSize   = 2;
        constexpr unsigned packetSize = DMX_PRO_HEADER_SIZE + dataSize + DMX_PRO_END_SIZE;
        std::vector<uint8_t> packet(packetSize);

        packet[0] = DMX_PRO_START_MSG;
        packet[1] = ENTTEC_PRO_PORT_ASS_REQ;
        packet[2] = static_cast<uint8_t>(dataSize & 0xFF);
        packet[3] = static_cast<uint8_t>((dataSize >> 8) & 0xFF);

        packet[4] = 1; // port 1 → DMX out universe 1
        packet[5] = 1; // port 2 → DMX out universe 2

        packet[packetSize - 1] = DMX_PRO_END_MSG;

        serial_.writeBytes(packet.data(), static_cast<int>(packetSize));
    }
    tc::sleepMillis(200);
    tc::logNotice("tcxDmx") << "MK2: enabled both DMX ports";

    universes_ = 2;
}

bool Dmx::badChannel(unsigned int channel) const {
    if (channel == 0) {
        tc::logError("tcxDmx") << "Channel 0 does not exist. DMX channels start at 1.";
        return true;
    }
    if (channel > levels_.size()) {
        tc::logError("tcxDmx") << "Channel " << channel
                               << " is out of bounds. Only " << levels_.size()
                               << " channels are available.";
        return true;
    }
    return false;
}

void Dmx::setLevel(unsigned int channel, uint8_t level, unsigned int universe) {
    if (badChannel(channel)) return;
    const auto idx = channel - 1;

    if (universe == 1) {
        if (levels_[idx] != level) {
            levels_[idx] = level;
            needsUpdate_ = true;
        }
    } else if (universe == 2) {
        if (levels2_[idx] != level) {
            levels2_[idx] = level;
            needsUpdate_ = true;
        }
    } else {
        tc::logError("tcxDmx") << "Universe " << universe << " not supported (use 1 or 2).";
    }
}

uint8_t Dmx::getLevel(unsigned int channel) const {
    if (channel == 0 || channel > levels_.size()) return 0;
    return levels_[channel - 1];
}

void Dmx::clear() {
    std::fill(levels_.begin(),  levels_.end(),  uint8_t{0});
    std::fill(levels2_.begin(), levels2_.end(), uint8_t{0});
    needsUpdate_ = true;
}

void Dmx::sendUniverse(int universeIndex) {
    const auto& src = (universeIndex == 0) ? levels_ : levels2_;

    const unsigned dataSize   = static_cast<unsigned>(src.size()) + DMX_START_CODE_SIZE;
    const unsigned packetSize = DMX_PRO_HEADER_SIZE + dataSize + DMX_PRO_END_SIZE;
    std::vector<uint8_t> packet(packetSize);

    packet[0] = DMX_PRO_START_MSG;
    packet[1] = (universeIndex == 0) ? DMX_PRO_SEND_PACKET : DMX_PRO_SEND_PACKET2;
    packet[2] = static_cast<uint8_t>(dataSize & 0xFF);
    packet[3] = static_cast<uint8_t>((dataSize >> 8) & 0xFF);
    packet[4] = DMX_START_CODE;
    std::copy(src.begin(), src.end(), packet.begin() + 5);
    packet[packetSize - 1] = DMX_PRO_END_MSG;

    serial_.writeBytes(packet.data(), static_cast<int>(packetSize));
}

void Dmx::update(bool force) {
    if (!connected_) return;
    if (!needsUpdate_ && !force) return;

    needsUpdate_ = false;
    for (int u = 0; u < universes_; ++u) {
        sendUniverse(u);
    }
}

} // namespace tcx
