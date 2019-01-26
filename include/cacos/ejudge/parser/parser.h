#pragma once

#include "cacos/ejudge/html/myhtml.h"
#include "cacos/ejudge/parser/task.h"

#include "cacos/config/config.h"
#include "cacos/ejudge/session.h"

#include <stdexcept>

namespace cacos::ejudge::parser {

class ParserError : public std::runtime_error {
public:
    ParserError(const std::string& what);
};

class Parser {
public:
    Parser(const config::Config& cfg);

    std::vector<Task> tasks() const;
    std::optional<Task> task(std::string_view key) const;
    i32 score() const;
    std::vector<Solution> solutions(i32 taskId) const;
    std::string_view source(i32 solutionId) const;

private:
    static constexpr std::string_view nbsp = "\xc2\xa0";

    mutable Session session_;
};

} // namespace cacos::ejudge::parser
