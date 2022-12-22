#include "json_builder.h"
#include <cassert>

namespace json {

    using ReturnType = Builder::ReturnType;
    using AfterKey = Builder::AfterKey;
    using AfterStartDict = Builder::AfterStartDict;
    using AfterStartArray = Builder::AfterStartArray;
    using AfterStartArrayValue = Builder::AfterStartArrayValue;

    ReturnType::ReturnType(Builder& builder) : builder_(builder){
    }
    AfterKey ReturnType::Key(const std::string& key) {
        builder_.Key(key);
        return {builder_};
    }
    Builder& ReturnType::EndDict() {
        builder_.EndDict();
        return builder_;
    }
    AfterStartDict ReturnType::StartDict() {
        builder_.StartDict();
        return {builder_};
    }
    AfterStartArray ReturnType::StartArray() {
        builder_.StartArray();
        return {builder_};
    }
    Builder& ReturnType::EndArray() {
        builder_.EndArray();
        return builder_;
    }


    AfterStartArrayValue AfterStartArray::Value(const Node& node) {
        builder_.Value(node);
        return {builder_};
    }

    AfterStartDict AfterKey::Value(const Node& node) {
        builder_.Value(node);
        return {builder_};
    }

    AfterStartArrayValue AfterStartArrayValue::Value(const Node& node) {
        builder_.Value(node);
        return {builder_};
    }


    Builder::Builder() {
        operations_.push_front(JBOperations::Builder);
    }
    Builder::AfterStartDict Builder::StartDict() {
        CheckAndCreateOperationOrder(JBOperations::StartDict);
        if(incompleteNodes_.empty()) {
            root_ = Dict {};
            incompleteNodes_.push_front(&root_.value());
        } else {
            Node *incompleteNode = incompleteNodes_.front();
            if(incompleteNode->IsArray()) {
                Array& arrayNode = std::get<Array>(incompleteNode->GetValue());
                arrayNode.push_back(Dict{});
                incompleteNodes_.push_front(&(arrayNode[arrayNode.size() - 1]));
            } else if(incompleteNode->IsDict()) {
                Dict& dictNode = std::get<Dict>(incompleteNode->GetValue());
                dictNode.insert({keys_.front(), Dict {}});
                incompleteNodes_.push_front(&dictNode[keys_.front()]);
                keys_.pop_front();
            } else {
                assert(false);
            }
        }
        return {*this};
    }
    Builder& Builder::EndDict() {
        CheckAndCreateOperationOrder(JBOperations::EndDict);
        incompleteNodes_.pop_front();
        return *this;
    }
    Builder::AfterStartArray Builder::StartArray() {
        CheckAndCreateOperationOrder(JBOperations::StartArray);
        if(incompleteNodes_.empty()) {
            root_ = Array {};
            incompleteNodes_.push_front(&root_.value());
        } else {
            Node *incompleteNode = incompleteNodes_.front();
            if(incompleteNode->IsArray()) {
                Array& arrayNode = std::get<Array>(incompleteNode->GetValue());
                arrayNode.push_back(Array {});
                incompleteNodes_.push_front(&(arrayNode[arrayNode.size() - 1]));
            } else if(incompleteNode->IsDict()) {
                Dict& dictNode = std::get<Dict>(incompleteNode->GetValue());
                dictNode.insert({keys_.front(), Array {}});
                incompleteNodes_.push_front(&dictNode[keys_.front()]);
                keys_.pop_front();
            } else {
                assert(false);
            }
        }
        return {*this};
    }
    Builder& Builder::EndArray() {
        CheckAndCreateOperationOrder(JBOperations::EndArray);
        incompleteNodes_.pop_front();
        return *this;
    }
    Builder& Builder::Key(const std::string& key) {
        CheckAndCreateOperationOrder(JBOperations::Key);
        keys_.push_front(key);
        return *this;
    }
    Builder& Builder::Value(const Node& node) {
        CheckAndCreateOperationOrder(JBOperations::Value);
        if(incompleteNodes_.empty() && operations_.size() == 2) {
            root_ = node;
        } else {
            Node *incompleteNode = incompleteNodes_.front();
            if(incompleteNode->IsArray()) {
                std::get<Array>(incompleteNode->GetValue()).push_back(node);
            } else if(incompleteNode->IsDict()) {
                std::get<Dict>(incompleteNode->GetValue()).insert({keys_.front(), node});
                keys_.pop_front();
            } else {
                assert(false);
            }
        }
        return *this;
    }
    Node& Builder::Build() {
        CheckAndCreateOperationOrder(JBOperations::Build);
        return root_.value();
    }

    std::string Builder::GetJBOperationName(const JBOperations& operation) {
        using namespace std::literals;
        switch (operation) {
            case JBOperations::Builder : return "Builder"s;
            case JBOperations::StartDict : return "StartDict"s;
            case JBOperations::EndDict : return "EndDict"s;
            case JBOperations::Key : return "Key"s;
            case JBOperations::Value : return "Value"s;
            case JBOperations::StartArray : return "StartArray"s;
            case JBOperations::EndArray : return "EndArray"s;
            case JBOperations::Build : return "Build"s;
            default: return "Unknown operation"s;
        }
    }
    void Builder::CheckJsonValue(const JBOperations& operation) {
        using namespace std::literals;
        if(incompleteNodes_.empty()) {
            if(operations_.front() != JBOperations::Builder) {
                throw std::logic_error("Wrong "s +
                                       GetJBOperationName(operation) + " command after "s +
                                       GetJBOperationName(operations_.front()) +
                                       ". Value command doesn't be used outside Array or Dict block, except for use immediately after constructor"s);
            }
        } else if(incompleteNodes_.front()->IsDict()) {
            if(operations_.front() != JBOperations::Key) {
                throw std::logic_error("Wrong "s +
                                       GetJBOperationName(operation) + " command after "s +
                                       GetJBOperationName(operations_.front()) +
                                       " inside Dict block."s);
            }
        }
        else if(!incompleteNodes_.front()->IsArray()){
            throw std::logic_error("Wrong "s +
                                   GetJBOperationName(operation) + " command after "s +
                                   GetJBOperationName(operations_.front()) +
                                   ". Value command doesn't be used outside Array or Dict block, except for use immediately after constructor"s);
        }
    }
    void Builder::CheckAndCreateOperationOrder(const JBOperations& operation) {
        using namespace std::literals;
        switch (operation) {
            case JBOperations::Key :
                if(incompleteNodes_.empty() || !incompleteNodes_.front()->IsDict()) {
                    throw std::logic_error("Wrong Key command. The Key command must be user only inside Dict block"s);
                } else if(operations_.front() == JBOperations::Key) {
                    throw std::logic_error("Wrong Key command. The Key command can't be used after another Key command"s);
                }; break;
            case JBOperations::Value :
                CheckJsonValue(JBOperations::Value);
                break;

            case JBOperations::StartDict :
                CheckJsonValue(JBOperations::StartDict);
                break;

            case JBOperations::StartArray :
                CheckJsonValue(JBOperations::StartArray);
                break;
            case JBOperations::EndDict :
                if(incompleteNodes_.empty() || !incompleteNodes_.front()->IsDict()) {
                    throw std::logic_error("Wrong EndDict command. EndDict must be used inside Dist block."s);
                }
                break;
            case JBOperations::EndArray :
                if(incompleteNodes_.empty() || !incompleteNodes_.front()->IsArray()) {
                    throw std::logic_error("Wrong EndArray command. EndArray must be used inside Array block."s);
                }
                break;
            case JBOperations::Build :
                if(operations_.size() == 1)  {
                    throw std::logic_error("Wrong Build command. Build mustn't be used immediately after constructor"s);
                } else if(!incompleteNodes_.empty()) {
                    throw std::logic_error("Wrong Build command. Build mustn't be used if an incomplete Array or Dict block are exists"s);
                }
                break;
            default:
                throw std::logic_error("Unknown command."s);
        }
        operations_.push_front(operation);
    }

}