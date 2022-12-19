#pragma once
#include "json.h"
#include <string>
#include <exception>
#include <optional>
#include <deque>

namespace json {



    class Builder {

    public:
        class AfterStartArray;
        class AfterKey;
        class AfterStartArrayValue;
        class AfterStartDict;

        class ReturnType {

        public:

            ReturnType(Builder& builder);
            AfterKey Key(const std::string& key);
            Builder& EndDict();
            AfterStartDict StartDict();
            AfterStartArray StartArray();
            Builder& EndArray();
        protected:
            Builder &builder_;
        };


        class AfterStartDict : public ReturnType {

        public:

            AfterStartDict StartDict() = delete;
            AfterStartArray StartArray() = delete;
            Builder& EndArray() = delete;

        };

        class AfterStartArray : public ReturnType {
        public:

            AfterStartArrayValue Value(const Node& node);

            AfterKey Key(const std::string& key) = delete;
            Builder& EndDict() = delete;

        };

        class AfterKey : public ReturnType {

        public:

            AfterStartDict Value(const Node& node);

            AfterKey Key(const std::string& key) = delete;
            Builder& EndDict() = delete;
        };

        class AfterStartArrayValue : public ReturnType {

        public:

            AfterStartArrayValue Value(const Node& node);

            AfterKey Key(const std::string& key) = delete;
            Builder& EndDict() = delete;
        };

        Builder();
        AfterStartDict StartDict();
        Builder& EndDict();
        AfterStartArray StartArray();
        Builder& EndArray();
        Builder& Key(const std::string& key);
        Builder& Value(const Node& node);
        Node& Build();

    private:
        enum class JBOperations {
            Builder,
            StartDict,
            EndDict,
            Key,
            Value,
            StartArray,
            EndArray,
            Build
        };

        std::string GetJBOperationName(const JBOperations& operation);
        void CheckJsonValue(const JBOperations& operation);
        void CheckAndCreateOperationOrder(const JBOperations& operation);

        std::optional<Node> root_;
        std::deque<Node*> incompleteNodes_;
        std::deque<JBOperations> operations_;
        std::deque<std::string> keys_;
    };

}