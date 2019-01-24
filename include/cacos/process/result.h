#pragma once

#include <string>

namespace cacos::process {
namespace status {
enum Status {
    OK,
    RE,
    TL,
    IL,
    ML,
    UNDEFINED,
};

std::string serialize(Status status);
}

struct Result {
    status::Status status;
    int returnCode;
};

} // namespace cacos::process
