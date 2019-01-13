#include "cacos/process/result.h"

#include <vector>

namespace cacos::process::status {

std::string serialize(Status status) {
    static const std::vector<std::string> serialized{ "OK", "RE", "TL", "ML", "UNDEFINED" };
    if (status >= serialized.size()) {
        return serialized.back();
    }
    return serialized[status];
}

} // namespace cacos::process::status
