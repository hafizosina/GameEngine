#pragma once
#include <string>
#include "utils/Serializer.hpp"

namespace Zhenzhu {

class DataLoader {
public:
    Json Load(const std::string& path);
};

} // namespace Zhenzhu
