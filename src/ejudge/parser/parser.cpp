#include "cacos/ejudge/parser/parser.h"

#include "cacos/ejudge/html/printer.h"

#include "cacos/util/ranges.h"

namespace cacos::ejudge::parser {

ParserError::ParserError(const std::string& what)
    : std::runtime_error(what) {
}

Parser::Parser(const config::Config& cfg)
    : session_(cfg) {
}

std::vector<Task> Parser::tasks() const {
    html::Html summary = session_.getPage("view-problem-summary");

    std::vector<Task> result;

    for (auto table : summary.attrs("class", "table")) {
        for (auto row : util::skip(*util::select(table, 1).begin(), 1)) {
            if (row.tagId() != MyHTML_TAG_TR) {
                continue;
            }

            std::vector<html::Node> children;
            for (auto child : row) {
                children.push_back(child);
            }
            if (children.size() != 13) {
                throw ParserError(
                    "Cannot parse ejudge responce: weird summary table size " +
                    util::string::to(children.size()));
            }

            auto get = [](auto&& opt) -> auto& {
                if (!opt) {
                    opt.emplace();
                }
                return *opt;
            };

            Task task;
            task.name = children[1].child()->text();
            auto link = children[3].child()->attr("href").value();
            size_t pos = link.find_first_of('?');
            if (pos != std::string_view::npos) {
                auto tokens = util::split(link.substr(pos + 1), "=&");
                for (auto [token, i] : util::enumerate(tokens)) {
                    if (token == "prob_id") {
                        task.id = util::string::from<i32>(tokens[i + 1]);
                    }
                }
            }
            auto status = children[5].child()->text();
            if (status != nbsp) {
                get(task.result).status = util::str(status);
            }
            auto score = children[9].child()->text();
            if (score != nbsp) {
                get(task.result).score = util::string::from<i32>(score);
            }

            result.push_back(std::move(task));
        }
    }

    return result;
}

std::optional<Task> Parser::task(std::string_view key) const {
    for (auto&& it : tasks()) {
        if (it.name == key) {
            return it;
        }
    }
    return std::nullopt;
}

i32 Parser::score() const {
    html::Html summary = session_.getPage("view-problem-summary");
    for (auto node : summary.tags(MyHTML_TAG__TEXT)) {
        if (util::string::starts(node.text(), "Total score:")) {
            std::string_view score = node.text().substr(std::string_view("Total score: ").size());
            try {
                return util::string::from<i32>(score);
            } catch (const std::exception&) {
                std::throw_with_nested(ParserError("Cannot find total score"));
            }
        }
    }
    throw ParserError("Cannot find total score");
}

namespace view_solution {
enum {
    ID = 0,
    RESULT = 5,
    TESTS_PASSED = 6,
    SCORE = 7,
};
}

std::vector<Solution> Parser::solutions(i32 taskId) const {
    html::Html page = session_.getPage(
        "view-problem-submit", util::string::join("prob_id=", util::string::to(taskId)));

    std::vector<Solution> result;

    for (auto table : page.attrs("class", "table")) {
        auto tbody = table.child().value();
        if (tbody.tag() != MyHTML_TAG_TBODY) {
            log::debug() << "tbody tag: " << tbody.tag();
        }
        for (auto row : util::skip(tbody, 1)) {
            log::debug() << std::hex << row.tag() << std::dec;
            if (row.tag() != MyHTML_TAG_TR) {
                continue;
            }

            Solution solution;
            for (auto [node, i] : util::enumerate(util::skip(row, 1))) {
                switch (i) {
                case view_solution::ID:
                    solution.id = util::string::from<i32>(node.innerText());
                    break;
                case view_solution::RESULT:
                    solution.result.status = node.innerText();
                    break;
                case view_solution::TESTS_PASSED:
                    try {
                        solution.result.tests = util::string::from<i32>(node.innerText());
                    } catch (...) {
                    }
                    break;
                case view_solution::SCORE:
                    try {
                        solution.result.score = util::string::from<i32>(node.innerText());
                    } catch (...) {
                    }
                    break;
                }
            }

            result.push_back(std::move(solution));
        }
    }

    return result;
}

std::string_view Parser::source(i32 solutionId) const {
    std::string_view result = session_.getRaw(
        "download-run", util::string::join("run_id=", util::string::to(solutionId)));

    html::Html page(result);
    for (auto node : page.tags("title")) {
        if (node.child()->text().find("Permission denied") != std::string_view::npos) {
            throw AuthenticationError("Invalid solution id");
        }
    }

    return result;
}

std::pair<html::Html, html::Node> Parser::statement(i32 taskId) const {
    html::Html page = session_.getPage(
        "view-problem-submit", util::string::join("prob_id=", util::string::to(taskId)));

    std::optional<html::Node> root;
    for (auto node : page.tags(MyHTML_TAG_H3)) {
        if (util::string::starts(node.innerText(), "Problem")) {
            root = node.next();
        }
    }

    if (!root) {
        throw std::runtime_error("Cannot find statement");
    }

    return {std::move(page), root.value()};
}

} // namespace cacos::ejudge::parser
