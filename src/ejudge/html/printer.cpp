#include "cacos/ejudge/html/printer.h"

#include "cacos/util/optional_ref.h"
#include "cacos/util/ranges.h"
#include "cacos/util/split.h"
#include "cacos/util/string.h"

#include "cacos/util/terminfo/terminfo.h"

#include <termcolor/termcolor.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <functional>

#include <utf8.h>

namespace cacos::html {

namespace tc = termcolor;

namespace {

class State {
public:
    enum {
        header = 1 << 1,
        code = 1 << 2,
    };

    ui64 flags() const {
        return flags_;
    }

    void flags(ui64 f) {
        flags_ = f;
    }

    bool flag(ui64 f) {
        return !!(f & flags_);
    }

    void toggleFlag(ui64 flag) {
        flags_ ^= flag;
    }

    void setFlag(ui64 flag) {
        flags_ |= flag;
    }

    void removeFlag(ui64 flag) {
        flags_ &= ~flag;
    }

private:
    ui64 flags_ = 0;
};

template<typename... Args>
class StyleSetter {
public:
    StyleSetter(std::ostream& os, Args&&... args)
        : os_(os) {
        (os_ << ... << args);
    }

    ~StyleSetter() {
        os_ << tc::reset;
    }

private:
    std::ostream& os_;
};

class TagPrinter {
public:
    virtual ~TagPrinter() = default;
    virtual void prefix(std::ostream&, Node, State&) const = 0;
    virtual void suffix(std::ostream&, Node, State&) const = 0;
};

namespace tags {

class Null : public TagPrinter {
public:
    void prefix(std::ostream&, Node, State&) const override {
    }

    void suffix(std::ostream&, Node, State&) const override {
    }
};

class Header : public TagPrinter {
public:
    void prefix(std::ostream& os, Node, State&) const override {
        tc::bold(os);
        fmt::print(os, "\n");
    }

    void suffix(std::ostream& os, Node, State&) const override {
        tc::reset(os);
        fmt::print(os, "\n");
    }
};

class Paragraph : public TagPrinter {
public:
    void prefix(std::ostream& os, Node, State&) const override {
        fmt::print(os, "\n");
    }

    void suffix(std::ostream& os, Node, State&) const override {
        fmt::print(os, "\n");
    }
};

class Code : public TagPrinter {
public:
    void prefix(std::ostream& os, Node, State& state) const override {
        fmt::print(os, "\n");
        state.setFlag(State::code);
    }

    void suffix(std::ostream&, Node, State& state) const override {
        state.removeFlag(State::code);
    }
};

class Text : public TagPrinter {
public:
    void prefix(std::ostream& os, Node node, State& state) const override {
        std::string text = util::str(node.text());
        if (!state.flag(State::code)) {
            for (char& c : text) {
                if (c == '\n') {
                    c = ' ';
                }
            }

            std::string tmp = std::move(text);
            text.clear();

            auto words = util::split(tmp, " ");

            text = util::str(words[0]);

            i64 length = utf8::distance(text.begin(), text.end());
            for (auto word : util::skip(words, 1)) {
                i64 wordLength = utf8::distance(word.begin(), word.end());
                if (length + wordLength + 1 < util::terminfo::get().width) {
                    text += " ";
                    ++length;
                } else {
                    text += "\n";
                    length = 0;
                }
                text += word;
                length += wordLength;
            }
        }
        fmt::print(os, "{}", text);
    }

    void suffix(std::ostream&, Node, State&) const override {
    }
};

Null null;

Code code;
Header header;
Paragraph paragraph;
Text text;

} // namespace tags

static std::unordered_map<myhtml_tag_id_t, TagPrinter&> tags_ = {
    {MyHTML_TAG_H1, tags::header},
    {MyHTML_TAG_H2, tags::header},
    {MyHTML_TAG_H3, tags::header},
    {MyHTML_TAG_H4, tags::header},
    {MyHTML_TAG_H5, tags::header},
    {MyHTML_TAG_H6, tags::header},
    {MyHTML_TAG_P, tags::paragraph},
    {MyHTML_TAG_PRE, tags::code},
    {MyHTML_TAG__TEXT, tags::text},
};

class Printer {
public:
    Printer(Node node, std::ostream& os, State& state)
        : node_(node)
        , os_(os)
        , state_(state) {
    }

    void start() {
        selectPrinter().prefix(os_, node_, state_);
    }

    void finish() {
        selectPrinter().suffix(os_, node_, state_);
    }

private:
    const TagPrinter& selectPrinter() const {
        auto it = tags_.find(node_.tagId());
        if (it != tags_.end()) {
            return it->second;
        }
        return tags::null;
    }

private:
    Node node_;
    std::ostream& os_;
    State& state_;
};

void printImpl(Node node, std::ostream& out, State& state) {
    std::string s = node.innerText();
    Printer printer(node, out, state);
    printer.start();
    for (auto child : node) {
        printImpl(child, out, state);
    }
    printer.finish();
}

} // namespace

void print(Node node, std::ostream& out) {
    State state;
    printImpl(node, out, state);
}

} // namespace cacos::html
