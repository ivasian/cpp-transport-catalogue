#include "json.h"

using namespace std;

namespace json {

    namespace {

        using Number = std::variant<int, double>;
        void PrintNode(const Node& node, std::ostream& out);

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char c;
            for (;input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if( c != ']')  {
                throw json::ParsingError("] is absent.");
            }
            return Node(result);
        }

        class ParsingError : public std::runtime_error {
        public:
            using runtime_error::runtime_error;
        };

        std::string EscapeCharacters(const std::string& str) {
            string result;
            for(char c : str) {
                switch (c) {
                    case '\\' : result += "\\\\"s; break;
                    case '\n' : result += "\\n"s; break;
                    case '\r' : result += "\\r"s; break;
                    case '"' : result += "\\\""s; break;
                    default : result.push_back(c);
                }
            }
            return result;
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node{std::stoi(parsed_num)};
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node{std::stod(parsed_num)};
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }
        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw json::ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            // Встретили неизвестную escape-последовательность
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }
            return Node(move(s));
        }
        Node LoadNull(std::istream& input) {
            char c;
            string result;
            size_t i = 0;
            while(i++ < 4 && input >> c) {
                result.push_back(c);
            }
            if(result != "null"s){
                throw json::ParsingError("Wrong null - "s + result);
            }
            return Node();
        }
        Node LoadDict(istream& input) {
            Dict result;
            char c;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }
            if( c != '}')  {
                throw json::ParsingError("} is absent.");
            }

            return Node(move(result));
        }
        Node LoadBool(istream& input){
            char c;
            input >> c;
            std::string word;
            word.push_back(c);
            if(c == 't'){
                for(size_t i = 0; i < 3; ++i){
                    input >> c;
                    word.push_back(c);
                }
                if(word != "true"s){
                    throw json::ParsingError("Bool parsing error - "s + word);
                }
                return Node(true);
            } else if(c == 'f') {
                for(size_t i = 0; i < 4; ++i){
                    input >> c;
                    word.push_back(c);
                }
                if(word != "false"s){
                    throw json::ParsingError("Bool parsing error - "s + word);
                }
                return Node(false);
            } else {
                throw ParsingError("Bool parsing error");
            }
        }
        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '}' || c == ']') {
                throw json::ParsingError("} ( or ]) without { (or [) before that"s);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 't') {
                input.putback(c);
                return LoadBool(input);
            } else if (c == 'f') {
                input.putback(c);
                return LoadBool(input);
            } else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            } else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

        void PrintValue(const int value, std::ostream& out) {
            out << value;
        }
        void PrintValue(const double value, std::ostream& out) {
            out << value;
        }
        void PrintValue(const std::string& value, std::ostream& out) {
            out << "\""sv << EscapeCharacters(value) << "\""sv;
        }
        void PrintValue(const bool value, std::ostream& out) {
            if(value){
                out << "true"sv;
            } else {
                out << "false"sv;
            }
        }
        void PrintValue(const Array& array, std::ostream& out) {
            out << "["sv;
            bool ifFirst = true;
            for(auto& node : array){
                if(!ifFirst) {
                    out << ", "sv;
                }
                PrintNode(node, out);
                ifFirst = false;
            }
            out << "]"sv;
        }
        void PrintValue(const Dict& array, std::ostream& out) {
            out << "{"sv;
            bool ifFirst = true;
            for(auto& [key, node] : array){
                if(!ifFirst) {
                    out << ", "sv;
                }
                out << "\""sv << key << "\": "sv;
                PrintNode(node, out);
                ifFirst = false;
            }
            out << "}"sv;
        }
        void PrintValue(std::nullptr_t, std::ostream& out) {
            out << "null"sv;
        }

        void PrintNode(const Node& node, std::ostream& out) {

            std::visit(
                    [&out](const auto& value){ PrintValue(value, out); },
                    node.GetValue());
        }


    }  // namespace

    const NodeValue& Node::GetValue() const {
        return *this;
    }

    const Array& Node::AsArray() const {
        if(!IsArray()) {
            throw std::logic_error("Getting value as Array's type but value type's isn't Array"s);
        }
        return get<Array>(*this);
    }
    const Dict& Node::AsMap() const {
        if(!IsMap()) {
            throw std::logic_error("Getting value as Dict's type but value type's isn't Dict"s);
        }
        return get<Dict>(*this);
    }
    int Node::AsInt() const {
        if(!IsInt()) {
            throw std::logic_error("Getting value as int's type but value type's isn't int"s);
        }
        return get<int>(*this);
    }
    double Node::AsDouble() const {
        if(!(IsDouble() || IsInt())) {
            throw std::logic_error("Getting value as int or double type but value type's isn't int or double"s);
        }
        if(IsPureDouble()){
            return get<double>(*this);
        }
        return static_cast<double>(get<int>(*this));

    }
    const string& Node::AsString() const {
        if(!IsString()) {
            throw std::logic_error("Getting value as string's type but value type's isn't string"s);
        }
        return get<std::string>(*this);
    }
    bool Node::AsBool() const {
        if(!IsBool()) {
            throw std::logic_error("Getting value as bool's type but value type's isn't bool"s);
        }
        return get<bool>(*this);
    }



    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(*this);
    }
    bool Node::IsInt() const {
        return holds_alternative<int>(*this);
    }
    bool Node::IsDouble() const {
        return holds_alternative<double>(*this) ||
               holds_alternative<int>(*this);
    }
    bool Node::IsPureDouble() const {
        return holds_alternative<double>(*this);
    }
    bool Node::IsString() const {
        return holds_alternative<string>(*this);
    }
    bool Node::IsArray() const {
        return holds_alternative<Array>(*this);
    }
    bool Node::IsMap() const {
        return holds_alternative<Dict>(*this);
    }
    bool Node::IsBool() const {
        return holds_alternative<bool>(*this);
    }

    bool Node::operator==(const Node& rhs) const {
        return static_cast<NodeValue>(*this) == static_cast<NodeValue>(rhs);
    }
    bool Node::operator!=(const Node& rhs) const {
        return !(*this == rhs);
    }

    Document::Document(Node root): root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& doc) const {
        return root_ == doc.root_;
    }
    bool Document::operator!=(const Document& doc) const {
        return !(*this == doc);
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }
    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

}  // namespace json