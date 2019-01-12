#include "cacos/util/inline_variables.h"
#include "cacos/util/string.h"

#include <map>
#include <vector>

namespace cacos {

namespace  {

static constexpr char PREFIX = '@';
static constexpr char OPEN_BRACE = '{';
static constexpr char CLOSE_BRACE = '}';

enum class ParsingState {
    DEFAULT = 0,
    PREFIX,
    VARIABLE,
    VARIABLE_BEGIN,
    VARIABLE_END,
};

struct ParsingValue {
    ParsingState state;
    char c;
};

bool operator<(ParsingValue lhs, ParsingValue rhs) {
    return std::tie(lhs.state, lhs.c) < std::tie(rhs.state, rhs.c);
}

class TransitionTable {
public:
    TransitionTable() = default;

    ParsingState next(ParsingValue current) const {
        auto it = tr_.find(current);
        if (it == tr_.end()) {
            return ParsingState::DEFAULT;
        } else {
            return it->second;
        }
    }

    ParsingState next(ParsingState state, char c) const {
        return next(ParsingValue{state, c});
    }

    void add(ParsingState from, char c, ParsingState to) {
        tr_[ParsingValue{from, c}] = to;
    }

    void add(ParsingState from, ParsingState to) {
        unsigned char c = 0;
        do {
            add(from, static_cast<char>(c), to);
        } while (++c);
    }

private:
    using Table = std::map<ParsingValue, ParsingState>;

    Table tr_;
};

TransitionTable initTransitionTable() {
    TransitionTable result;

    result.add(ParsingState::VARIABLE_END, ParsingState::DEFAULT);
    result.add(ParsingState::VARIABLE_END, PREFIX, ParsingState::PREFIX);
    result.add(ParsingState::DEFAULT, PREFIX, ParsingState::PREFIX);
    result.add(ParsingState::PREFIX, OPEN_BRACE, ParsingState::VARIABLE_BEGIN);
    result.add(ParsingState::VARIABLE_BEGIN, ParsingState::VARIABLE);
    result.add(ParsingState::VARIABLE_BEGIN, CLOSE_BRACE, ParsingState::VARIABLE_END);
    result.add(ParsingState::VARIABLE, ParsingState::VARIABLE);
    result.add(ParsingState::VARIABLE, CLOSE_BRACE, ParsingState::VARIABLE_END);

    return result;
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
    static TransitionTable table = initTransitionTable();

    ParsingState state = ParsingState::DEFAULT;
    std::vector<std::pair<size_t, size_t>> vars;
    size_t firstPos = std::string::npos;
    for (size_t i = 0; i < str.size(); ++i) {
        state = table.next(state, str[i]);
        switch (state) {
        case ParsingState::VARIABLE_BEGIN:
            if (i == 0) {
                throw std::out_of_range("Invalid index");
            }
            firstPos = i - 1;
            break;
        case ParsingState::VARIABLE_END:
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
        std::string_view keyView = str.substr(l + 2, r - l - 2);
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
