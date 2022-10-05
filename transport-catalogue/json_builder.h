#pragma once
#include "json.h"
#include <string>
#include <exception>
#include <optional>
#include <deque>

namespace json {

    class AfterKey;
    class Builder;
    class AfterStartArrayValue;

    class AfterKeyValue {

    public:
        AfterKeyValue(Builder& builder);
        AfterKey Key(const std::string& key);
        Builder& EndDict();

    private:
        Builder &builder_;
    };

    class AfterStartDict {

    public:
        AfterStartDict(Builder& builder);
        AfterKey Key(const std::string& key);
        Builder& EndDict();

    private:
        Builder &builder_;
    };

    class AfterStartArray {

    public:
        AfterStartArray(Builder& builder);
        AfterStartArrayValue Value(const Node& node);
        AfterStartDict StartDict();
        AfterStartArray& StartArray();
        Builder& EndArray();

    private:
        Builder &builder_;
    };

    class AfterKey {

    public:
        AfterKey(Builder& builder);
        AfterKeyValue Value(const Node& node);
        AfterStartDict StartDict();
        AfterStartArray StartArray();

    private:
        Builder &builder_;
    };

    class AfterStartArrayValue {

    public:
        AfterStartArrayValue(Builder& builder);
        AfterStartArrayValue& Value(const Node& node);
        AfterStartDict StartDict();
        AfterStartArray StartArray();
        Builder& EndArray();

    private:
        Builder &builder_;
    };

    class Builder {

    public:
        Builder();
        AfterStartDict StartDict();
        Builder& EndDict();
        AfterStartArray StartArray();
        Builder& EndArray();
        Builder& Key(const std::string& key);
        Builder& Value(const Node& node);
        Node& Build();

    private:
        enum class JBOperations {Builder, StartDict, EndDict, Key, Value, StartArray, EndArray, Build};
        std::string GetJBOperationName(const JBOperations& operation);
        void CheckJsonValue(const JBOperations& operation);
        void CheckAndCreateOperationOrder(const JBOperations& operation);

        std::optional<Node> root_;
        std::deque<Node*> incompleteNodes_;
        std::deque<JBOperations> operations_;
        std::deque<std::string> keys_;
    };

}