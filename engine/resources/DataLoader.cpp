#include "resources/DataLoader.hpp"
#include "utils/Logger.hpp"

namespace Zhenzhu {

Json DataLoader::Load(const std::string& path) {
    return Serializer::LoadFile(path);
}

} // namespace Zhenzhu
