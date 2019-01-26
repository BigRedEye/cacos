#include "cacos/ejudge/html/myhtml.h"

#include "cacos/util/string.h"

#include <algorithm>
#include <stdexcept>

#include <iostream>

namespace cacos::html {

class MyHtml {
public:
    MyHtml(const MyHtml&) = delete;
    MyHtml(MyHtml&&);

    MyHtml& operator=(const MyHtml&) = delete;
    MyHtml& operator=(MyHtml&&);

    ~MyHtml();

    static myhtml_t* handler();

private:
    MyHtml();

private:
    myhtml_t* myhtml_ = nullptr;
};

MyHtml::MyHtml()
    : myhtml_(myhtml_create()) {
    myhtml_init(myhtml_, MyHTML_OPTIONS_DEFAULT, 1, 0);
}

MyHtml::MyHtml(MyHtml&& other) {
    *this = std::move(other);
}

MyHtml& MyHtml::operator=(MyHtml&& other) {
    std::swap(other.myhtml_, myhtml_);
    return *this;
}

MyHtml::~MyHtml() {
    myhtml_destroy(myhtml_);
}

myhtml_t* MyHtml::handler() {
    static MyHtml html;
    return html.myhtml_;
}

Attributes::Attributes(myhtml_tree_attr_t* first, myhtml_tree_attr_t* last)
    : first_(first)
    , last_(last) {
}

Attributes::Iterator Attributes::begin() const {
    return first_;
}

Attributes::Iterator Attributes::end() const {
    return last_ ? ++Iterator{last_} : last_;
}

Attributes::Iterator::Iterator(myhtml_tree_attr_t* attr)
    : attr_(attr) {
}

Attributes::Iterator& Attributes::Iterator::operator++() {
    attr_ = myhtml_attribute_next(attr_);
    return *this;
}

Attributes::Iterator Attributes::Iterator::operator++(int) {
    Iterator tmp = *this;
    ++*this;
    return tmp;
}

Attribute Attributes::Iterator::operator*() const {
    size_t length;
    const char* data = myhtml_attribute_key(attr_, &length);
    std::string_view key(data, length);

    data = myhtml_attribute_value(attr_, &length);
    std::string_view value(data, length);

    return {key, value};
}

bool Attributes::Iterator::operator!=(Iterator other) const {
    return attr_ != other.attr_;
}

Node::Node(myhtml_tree_node_t* node)
    : node_(node)
{}

Node::Iterator Node::begin() const {
    return myhtml_node_child(node_);
}

Node::Iterator Node::end() const {
    auto it = myhtml_node_last_child(node_);
    return it ? ++Iterator{it} : it;
}

std::optional<Node> Node::child() const {
    if ((*begin()).node_) {
        return *begin();
    }
    return {};
}

myhtml_tag_id_t Node::tag() const {
    return myhtml_node_tag_id(node_);
}

std::optional<std::string_view> Node::attr(std::string_view key) {
    for (auto a : attrs()) {
        if (a.key == key) {
            return a.value;
        }
    }
    return {};
}

std::string_view Node::text() const {
    size_t len = 0;
    const char* buf = myhtml_node_text(node_, &len);
    return {buf, len};
}

std::string Node::innerText() const {
    std::string result = util::str(text());
    for (auto node : *this) {
        result += node.innerText();
    }
    return result;
}

Node::Iterator::Iterator(myhtml_tree_node_t* node)
    : node_(node) {
}

Node::Iterator& Node::Iterator::operator++() {
    node_ = myhtml_node_next(node_);
    return *this;
}

Node::Iterator Node::Iterator::operator++(int) {
    Iterator tmp = *this;
    ++*this;
    return tmp;
}

Node Node::Iterator::operator*() const {
    return node_;
}

bool Node::Iterator::operator!=(Iterator other) const {
    return node_ != other.node_;
}

Attributes Node::attrs() const {
    return {
        myhtml_node_attribute_first(node_),
        myhtml_node_attribute_last(node_)
    };
}

Collection::Iterator::Iterator(const myhtml_collection_t* base, size_t pos)
    : pos_(pos)
    , base_(base) {
}

Collection::Iterator& Collection::Iterator::operator++() {
    ++pos_;
    return *this;
}

Collection::Iterator Collection::Iterator::operator++(int) {
    Iterator tmp = *this;
    ++pos_;
    return tmp;
}

Node Collection::Iterator::operator*() const {
    if (!base_) {
        throw std::runtime_error("Cannot access base collection at nullptr");
    }
    return base_->list[pos_];
}

bool Collection::Iterator::operator!=(Iterator other) const {
    return pos_ != other.pos_ || base_ != other.base_;
}

Collection::Collection(myhtml_collection_t* raw)
    : collection_(raw) {
}

Collection::Collection(Collection&& other) {
    *this = std::move(other);
}

Collection& Collection::operator=(Collection&& other) {
    std::swap(collection_, other.collection_);
    return *this;
}

Collection::~Collection() {
    myhtml_collection_destroy(collection_);
}

const myhtml_collection_t* Collection::raw() const {
    return collection_;
}

Collection::Iterator Collection::begin() const {
    return Iterator(raw(), 0);
}

Collection::Iterator Collection::end() const {
    return Iterator(raw(), raw()->length);
}

Html::Html()
    : tree_(myhtml_tree_create()) {
    myhtml_tree_init(tree_, MyHtml::handler());
}

Html::Html(std::string_view html)
    : Html() {
    myhtml_parse(tree_, MyENCODING_UTF_8, html.data(), html.size());
}

Html::Html(Html&& other) {
    *this = std::move(other);
}

Html& Html::operator=(Html&& other) {
    std::swap(tree_, other.tree_);
    return *this;
}

Html::~Html() {
    myhtml_tree_destroy(tree_);
}

Collection Html::tags(myhtml_tag_id_t tag) const {
    return myhtml_get_nodes_by_tag_id(tree_, nullptr, tag, nullptr);
}

Collection Html::tags(std::string_view name) const {
    return myhtml_get_nodes_by_name(tree_, nullptr, name.data(), name.size(), nullptr);
}

Collection Html::attrs(std::string_view name) const {
    return myhtml_get_nodes_by_attribute_key(tree_, nullptr, nullptr, name.data(), name.size(), nullptr);
}

Collection Html::attrs(std::string_view key, std::string_view value) const {
    return myhtml_get_nodes_by_attribute_value(tree_, nullptr, nullptr, true, key.data(), key.size(), value.data(), value.size(), nullptr);
}

}
