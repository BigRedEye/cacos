#pragma once

#include <dtl/dtl.hpp>

#include <memory>
#include <string>
#include <string_view>

namespace cacos::util::diff {

class Unified {
public:
    Unified(const std::string& left, const std::string& right);

    void print(std::ostream& os) const;

    bool empty() const;

private:
    std::vector<dtl::uniHunk<std::pair<std::string, dtl::eleminfo>>> hunks_;
};

} // namespace cacos::util::diff
