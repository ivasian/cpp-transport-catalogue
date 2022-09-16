#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {


    using Color = std::string;
    inline const Color NoneColor{"none"};

    class Object;

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<< (std::ostream& out, const StrokeLineJoin& strokeLineJoin);
    std::ostream& operator<< (std::ostream& out, const StrokeLineCap& StrokeLineCap);

    class ObjectContainer {

    public:
        virtual ~ObjectContainer() = default;

        template<typename Obj>
        void Add(Obj object);

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        std::vector<std::unique_ptr<Object>> objects;
    };

    class Drawable {
    public:
        virtual ~Drawable() = default;
        virtual void Draw(ObjectContainer& container) const = 0;
    };

    struct Point {
        double x = 0;
        double y = 0;
    };

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
    struct RenderContext {
        RenderContext(std::ostream& out);
        RenderContext(std::ostream& out, int indent_step, int indent = 0);
        RenderContext Indented() const;
        void RenderIndent() const;

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

/*
 * Вспомогательный базовый класс svg::PathProps . Путь — представленный в виде последовательности
 * различных контуров векторный объект, который будет содержать свойства, управляющие параметрами
 * заливки и контура.
 */
    template<typename  Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color);
        Owner& SetStrokeColor(Color color);
        Owner& SetStrokeWidth(double width);
        Owner& SetStrokeLineCap(StrokeLineCap line_cap);
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

        void RenderAvailableColorsTags(std::ostream& out) const;
    protected:
        std::optional<Color> fillColor_;
        std::optional<Color> strokeColor_;
        std::optional<double> strokeWidth_;
        std::optional<StrokeLineCap> strokeLineCap_;
        std::optional<StrokeLineJoin> strokeLineJoin_;
    private:
        Owner& AsOwner();
    };

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

        void RenderObject(const RenderContext& context) const override;
        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        std::vector<Point> points;
    };

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
    class Text : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string fontFamily);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string fontWeight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

        // Прочие данные и методы, необходимые для реализации элемента <text>
        void RenderObject(const RenderContext &context) const override;
    private:
        std::string EscapeСharacters(std::string& data);

        Point position_ = {0.0, 0.0};
        Point offset_ = {0.0, 0.0};
        uint32_t fontSize_ = 1;
        std::string fontWeigh_;
        std::string fontFamily_;
        std::string data_;
    };

    class Document : public ObjectContainer{
    public:

        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    };

    template<typename Obj>
    void ObjectContainer::Add(Obj object) {
        objects.push_back(std::make_unique<Obj>(std::move(object)));
    }

    template<typename Owner>
    Owner& PathProps<Owner>::SetFillColor(Color color) {
        fillColor_ = color;
        return AsOwner();
    }

    template<typename Owner>
    Owner& PathProps<Owner>::SetStrokeColor(Color color) {
        strokeColor_ = color;
        return AsOwner();
    }

    template<typename Owner>
    Owner& PathProps<Owner>::SetStrokeWidth(double width) {
        strokeWidth_ = width;
        return AsOwner();
    }

    template<typename Owner>
    Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
        strokeLineCap_ = line_cap;
        return AsOwner();
    }

    template<typename Owner>
    Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
        strokeLineJoin_ = line_join;
        return AsOwner();
    }

    template<typename Owner>
    void PathProps<Owner>::RenderAvailableColorsTags(std::ostream& out) const {
        using namespace std::literals;
        if(fillColor_){
            out << " fill=\""s << fillColor_.value() <<"\""s;
        }
        if(strokeColor_){
            out << " stroke=\""s << strokeColor_.value() <<"\""s;
        }
        if(strokeWidth_){
            out << " stroke-width=\""s << strokeWidth_.value() <<"\""s;
        }
        if(strokeLineCap_){
            out << " stroke-linecap=\""s << strokeLineCap_.value() <<"\""s;
        }
        if(strokeLineJoin_){
            out << " stroke-linejoin=\""s << strokeLineJoin_.value() <<"\""s;
        }
    }

    template<typename Owner>
    Owner& PathProps<Owner>::AsOwner(){
        return static_cast<Owner&>(*this);
    }



}  // namespace svg