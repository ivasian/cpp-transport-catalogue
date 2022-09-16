#include "svg.h"

namespace svg {

    using namespace std::literals;

    RenderContext::RenderContext(std::ostream& out)
    : out(out) {
    }

    RenderContext::RenderContext(std::ostream& out, int indent_step, int indent)
        : out(out) , indent_step(indent_step) , indent(indent) {
    }

    RenderContext RenderContext::Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderContext::RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }


    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        // Делегируем вывод тега своим подклассам
        RenderObject(context);
        context.out << std::endl;
    }

// ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAvailableColorsTags(out);
        out << "/>"sv;
    }

// ---------- Document ----------------

    void Document::Render(std::ostream& out) const{
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for(auto& object : objects){
            object->Render(ctx);
        }
        out << "</svg>"sv;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects.push_back(std::move(obj));
    }

// ---------- Polyline -----------------

    Polyline& Polyline::AddPoint(Point point){
        points.emplace_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const{
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool if_first = true;
        for(auto& point : points){
            if(!if_first){
                out << ' ';
            } else {
                if_first = false;
            }
            out << point.x << ","sv << point.y;

        }
        out << "\""sv;
        RenderAvailableColorsTags(out);
        out << "/>"sv;
    }

// ------------- Text -----------------
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        fontSize_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string fontFamily) {
        fontFamily_ = std::move(fontFamily);
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string fontWeight) {
        fontWeigh_ = std::move(fontWeight);
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data){
        data_ = std::move(Text::EscapeСharacters(data));
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y <<
            "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv
            << fontSize_ << "\""sv;
        if(!fontFamily_.empty()) {
            out << " font-family=\""sv << fontFamily_ << "\""sv;
        }
        if(!fontWeigh_.empty()) {
            out << " font-weight=\""sv << fontWeigh_ << "\""sv;
        }
        RenderAvailableColorsTags(out);
        out << ">"sv << data_ << "</text>"sv;
    }

    std::string Text::EscapeСharacters(std::string& data) {
        std::string result;
        for(char c : data){
            if(c == '"'){
                result += "&quot;"s;
            } else if(c == '\'') {
                result += "&apos;"s;
            } else if(c == '<') {
                result += "&lt;"s;
            } else if(c == '>') {
                result += "&gt;"s;
            } else if(c == '&') {
                result += "&amp;"s;
            } else {
                result.push_back(c);
            }
        }
        return result;
    }

    std::ostream& operator<< (std::ostream& out, const StrokeLineJoin& strokeLineJoin) {
        switch (strokeLineJoin) {
            case StrokeLineJoin::ARCS : out << "arcs"; break;
            case StrokeLineJoin::BEVEL : out << "bevel"; break;
            case StrokeLineJoin::MITER : out << "miter"; break;
            case StrokeLineJoin::MITER_CLIP : out << "miter-clip"; break;
            case StrokeLineJoin::ROUND : out << "round"; break;
            default: out << "StrokeLineJoin value don't exists";
        }
        return out;
    }

    std::ostream& operator<< (std::ostream& out, const StrokeLineCap& StrokeLineCap) {
        switch (StrokeLineCap) {
            case StrokeLineCap::BUTT : out << "butt"; break;
            case StrokeLineCap::ROUND : out << "round"; break;
            case StrokeLineCap::SQUARE : out << "square"; break;
            default: out << "StrokeLineCap value don't exists";
        }
        return out;
    }

}  // namespace svg