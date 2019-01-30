#include "cacos/util/optional_ref.h"

namespace cacos::util {

bad_optional_ref_access::bad_optional_ref_access()
    : std::runtime_error("Null optional ref access") {
}

} // namespace cacos::util
