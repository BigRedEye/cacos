#include "cacos/util/map.h"

namespace cacos::util {

MappingError::MappingError(const std::string& what)
    : std::runtime_error(what) {
}

} // namespace cacos::util
