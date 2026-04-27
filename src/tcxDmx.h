#pragma once

#include <TrussC.h>

#include <cstdint>
#include <string>
#include <vector>

namespace tcx {

class Dmx {
public:
    Dmx();
    ~Dmx();

    Dmx(const Dmx&) = delete;
    Dmx& operator=(const Dmx&) = delete;

    bool connect(int deviceIndex = 0, unsigned int channels = 24);
    bool connect(const std::string& devicePath, unsigned int channels = 24);
    void disconnect();
    bool isConnected() const { return connected_; }

    void activateMk2(uint8_t key0 = 0xC8, uint8_t key1 = 0xD0,
                     uint8_t key2 = 0x88, uint8_t key3 = 0xAD);

    void setLevel(unsigned int channel, uint8_t level, unsigned int universe = 1);
    uint8_t getLevel(unsigned int channel) const;
    void clear();

    void setChannels(unsigned int channels = 24);
    void update(bool force = false);

private:
    trussc::Serial       serial_;
    std::vector<uint8_t> levels_;
    std::vector<uint8_t> levels2_;
    int                  universes_   = 1;
    bool                 connected_   = false;
    bool                 needsUpdate_ = false;

    bool badChannel(unsigned int channel) const;
    void sendUniverse(int universeIndex);
};

} // namespace tcx
