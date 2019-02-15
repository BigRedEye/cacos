#include "cacos/ejudge/html/printer.h"

#include "cacos/util/optional_ref.h"

#include "termcolor/termcolor.hpp"

#include "fmt/format.h"
#include "fmt/ostream.h"

#include <functional>

namespace cacos::html {

namespace tc = termcolor;

namespace {

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
    virtual void prefix(std::ostream&, Node) const = 0;
    virtual void suffix(std::ostream&, Node) const = 0;
};

namespace tags {

class Null : public TagPrinter {
public:
    void prefix(std::ostream&, Node) const override {
    }

    void suffix(std::ostream&, Node) const override {
    }
};

class Header : public TagPrinter {
public:
    void prefix(std::ostream& os, Node) const override {
        tc::bold(os);
        fmt::print(os, "\n");
    }

    void suffix(std::ostream& os, Node) const override {
        tc::reset(os);
        fmt::print(os, "\n");
    }
};

class Paragraph : public TagPrinter {
public:
    void prefix(std::ostream& os, Node) const override {
        fmt::print(os, "\n");
    }

    void suffix(std::ostream& os, Node) const override {
        fmt::print(os, "\n");
    }
};

class Code : public TagPrinter {
public:
    void prefix(std::ostream& os, Node) const override {
        fmt::print(os, "\n");
    }

    void suffix(std::ostream& os, Node) const override {
        fmt::print(os, "\n");
    }
};

class Text : public TagPrinter {
public:
    void prefix(std::ostream& os, Node node) const override {
        fmt::print(os, node.text());
    }

    void suffix(std::ostream&, Node) const override {
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
    Printer(Node node, std::ostream& os)
        : node_(node)
        , os_(os) {
    }

    void start() {
        selectPrinter().prefix(os_, node_);
    }

    void finish() {
        selectPrinter().suffix(os_, node_);
    }

private:
    const TagPrinter& selectPrinter() const {
        auto tag = node_.tag();
        auto it = tags_.find(node_.tagId());
        if (it != tags_.end()) {
            return it->second;
        }
        return tags::null;
    }

private:
    Node node_;
    std::ostream& os_;
};

} // namespace

void print(Node node, std::ostream& out) {
    std::string s = node.innerText();
    Printer printer(node, out);
    printer.start();
    for (auto child : node) {
        print(child, out);
    }
    printer.finish();
}

} // namespace cacos::html
