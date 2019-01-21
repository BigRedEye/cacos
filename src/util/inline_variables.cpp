#include "cacos/util/inline_variables.h"
#include "cacos/util/string.h"

#include <map>
#include <unordered_map>
#include <vector>

namespace cacos {

namespace  {

static constexpr char OPEN_BRACE = '{';
static constexpr char CLOSE_BRACE = '}';

namespace state {
enum ParsingState : ui64 {
    DEFAULT = 0,
    VARIABLE,
    VARIABLE_BEGIN,
    VARIABLE_END,
    PREFIX,
};
}

struct ParsingValue {
    state::ParsingState state;
    char c;
};

bool operator<(ParsingValue lhs, ParsingValue rhs) {
    return std::tie(lhs.state, lhs.c) < std::tie(rhs.state, rhs.c);
}

class TransitionTable {
public:
    TransitionTable() = default;

    state::ParsingState next(ParsingValue current) const {
        auto it = tr_.find(current);
        if (it == tr_.end()) {
            return state::DEFAULT;
        } else {
            return it->second;
        }
    }

    state::ParsingState next(state::ParsingState state, char c) const {
        return next(ParsingValue{state, c});
    }

    void add(state::ParsingState from, char c, state::ParsingState to) {
        tr_[ParsingValue{from, c}] = to;
    }

    void add(state::ParsingState from, state::ParsingState to) {
        unsigned char c = 0;
        do {
            add(from, static_cast<char>(c), to);
        } while (++c);
    }

private:
    using Table = std::map<ParsingValue, state::ParsingState>;

    Table tr_;
};

TransitionTable initTransitionTable(std::string_view prefix) {
    TransitionTable result;

    result.add(state::VARIABLE_END, state::DEFAULT);
    result.add(state::VARIABLE_END, prefix[0], static_cast<state::ParsingState>(state::PREFIX + 1));
    result.add(state::DEFAULT, prefix[0], static_cast<state::ParsingState>(state::PREFIX + 1));

    for (size_t i = 1; i < prefix.size(); ++i) {
        result.add(
            static_cast<state::ParsingState>(state::PREFIX + i),
            prefix[i],
            static_cast<state::ParsingState>(state::PREFIX + i + 1));
    }

    result.add(
        static_cast<state::ParsingState>(state::PREFIX + prefix.size()),
        OPEN_BRACE,
        state::VARIABLE_BEGIN
    );

    result.add(state::VARIABLE_BEGIN, state::VARIABLE);
    result.add(state::VARIABLE_BEGIN, CLOSE_BRACE, state::VARIABLE_END);
    result.add(state::VARIABLE, state::VARIABLE);
    result.add(state::VARIABLE, CLOSE_BRACE, state::VARIABLE_END);

    return result;
}

const TransitionTable& tableForPrefix(std::string_view prefix) {
    static std::unordered_map<std::string, TransitionTable> tables;

    auto it = tables.find(util::str(prefix));
    if (it == tables.end()) {
        it = tables.emplace(util::str(prefix), initTransitionTable(prefix)).first;
    }

    return it->second;
}

}

UnknownVariableName::UnknownVariableName(const std::string& name)
    : std::runtime_error(std::string("Unknown variable name:" + name))
{}

InlineVariableParsingError::InlineVariableParsingError(const std::string& what)
    : std::runtime_error(what)
{}

void InlineVariables::set(const std::string& key, const std::string& value) {
    vars_[key] = value;
}

std::string InlineVariables::parse(std::string_view str) const {
    const TransitionTable& table = tableForPrefix(prefix_);

    state::ParsingState state = state::DEFAULT;
    std::vector<std::pair<size_t, size_t>> vars;
    size_t firstPos = std::string::npos;
    for (size_t i = 0; i < str.size(); ++i) {
        state = table.next(state, str[i]);
        switch (state) {
        case state::VARIABLE_BEGIN:
            if (i < prefix_.size()) {
                throw std::out_of_range("Invalid index");
            }
            firstPos = i - prefix_.size();
            break;
        case state::VARIABLE_END:
            if (firstPos == std::string::npos) {
                InlineVariableParsingError("Cannot parse inline variables");
            }
            vars.emplace_back(firstPos, i);
            firstPos = std::string::npos;
            break;
        default:
            break;
        }
    }

    std::string result = util::str(str);
    for (auto it = vars.rbegin(); it != vars.rend(); ++it) {
        auto [l, r] = *it;
        size_t keyBegin = l + prefix_.length() + 1;
        std::string_view keyView = str.substr(keyBegin, r - keyBegin);
        std::string key = util::str(keyView);
        auto var = vars_.find(key);
        if (var == vars_.end()) {
            if (policy_ == UnknownVariablePolicy::IGNORE) {
                result.replace(l, r - l + 1, "");
            } else {
                throw UnknownVariableName(key);
            }
        } else {
            result.replace(l, r - l + 1, var->second);
        }
    }

    return result;
}

} // namespace cacos
