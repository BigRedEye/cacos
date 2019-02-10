#pragma once

#include <myhtml/api.h>

#include <optional>
#include <string_view>

namespace cacos::html {

struct Attribute {
    std::string_view key;
    std::string_view value;
};

class Attributes {
public:
    class Iterator;

    Attributes(myhtml_tree_attr_t* first, myhtml_tree_attr_t* last);

    Iterator begin() const;
    Iterator end() const;

private:
    myhtml_tree_attr_t* first_;
    myhtml_tree_attr_t* last_;
};

class Attributes::Iterator {
public:
    Iterator(myhtml_tree_attr_t* attr);

    Iterator& operator++();
    const Iterator operator++(int);

    bool operator!=(Iterator other) const;
    Attribute operator*() const;

private:
    myhtml_tree_attr_t* attr_;
};

class Node {
public:
    class Iterator;

    Node(myhtml_tree_node_t* node);

    Iterator begin() const;
    Iterator end() const;
    std::optional<Node> child() const;
    myhtml_tag_id_t tag() const;

    std::optional<std::string_view> attr(std::string_view key);

    std::string_view text() const;
    std::string innerText() const;
    Attributes attrs() const;

private:
    myhtml_tree_node_t* node_;
};

class Node::Iterator {
public:
    Iterator(myhtml_tree_node_t* node);

    Iterator& operator++();
    const Iterator operator++(int);

    bool operator!=(Iterator other) const;
    Node operator*() const;

private:
    myhtml_tree_node_t* node_;
};

class Collection {
public:
    class Iterator;

    Collection() = default;
    Collection(myhtml_collection_t* raw);

    Collection(const Collection&) = delete;
    Collection(Collection&&);

    Collection& operator=(const Collection&) = delete;
    Collection& operator=(Collection&&);

    ~Collection();

    const myhtml_collection_t* raw() const;

    Iterator begin() const;
    Iterator end() const;

private:
    myhtml_collection_t* collection_ = nullptr;
};

class Collection::Iterator {
public:
    Iterator(const myhtml_collection_t* base, size_t pos);

    Iterator& operator++();
    const Iterator operator++(int);

    Node operator*() const;

    bool operator!=(Iterator other) const;

private:
    size_t pos_ = 0;
    const myhtml_collection_t* base_ = nullptr;
};

class Html {
public:
    Html();
    Html(std::string_view html);

    Html(const Html&) = delete;
    Html(Html&&);

    Html& operator=(const Html&) = delete;
    Html& operator=(Html&&);

    ~Html();

    Collection tags(myhtml_tag_id_t tag) const;
    Collection tags(std::string_view name) const;
    Collection attrs(std::string_view key) const;
    Collection attrs(std::string_view key, std::string_view value) const;

private:
    myhtml_tree_t* tree_ = nullptr;
};

} // namespace cacos::html
