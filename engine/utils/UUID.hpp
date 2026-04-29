#pragma once
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstdint>

namespace Zhenzhu {

class UUID {
public:
    static std::string Generate() {
        static std::mt19937_64 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist;

        uint64_t hi = dist(rng);
        uint64_t lo = dist(rng);

        // format as xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
        std::ostringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(8)  << (uint32_t)(hi >> 32)         << "-"
           << std::setw(4)  << (uint16_t)((hi >> 16) & 0xFFFF) << "-"
           << std::setw(4)  << (uint16_t)(hi & 0xFFFF)       << "-"
           << std::setw(4)  << (uint16_t)(lo >> 48)          << "-"
           << std::setw(12) << (uint64_t)(lo & 0xFFFFFFFFFFFF);
        return ss.str();
    }
};

} // namespace Zhenzhu
