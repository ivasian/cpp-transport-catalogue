#pragma once
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>

#include "geo.h"
#include "svg.h"
#include "domain.h"


/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
using geo::Coordinates;
using svg::Point;
using svg::Polyline;
using svg::Circle;
using svg::Text;
using svg::Color;
using svg::StrokeLineJoin;
using svg::StrokeLineCap;

namespace renderer {

    struct RenderSettings {

        double width_;
        double height_;
        double padding_;
        double line_width_;
        double stop_radius_;
        int bus_label_font_size_;
        std::pair<double, double> bus_label_offset_;
        int stop_label_font_size_;
        std::pair<double, double>  stop_label_offset_;
        svg::Color underlayer_color_;
        double underlayer_width_;
        std::vector<svg::Color> color_palette_;

    };

    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding);

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class MapRenderer {

    public:
        MapRenderer(RenderSettings& renderSettings);

        SphereProjector GetSphereProjector(const std::vector<Coordinates>& allPoint) const;
        svg::Color GetStrokeColorByOrder(int number) const;
        const RenderSettings& GetRenderSettings() const;

        void SetLineProperties(Polyline& line, int number) const;
        void SetBusTextCommonProperties(Text& text,const std::string& data, Point position) const;
        void SetBusTextSubstrateProperties(Text& text,const std::string& data, Point position) const;
        void SetBusTextTitleProperties(Text& text,const std::string& data, const Color& color, Point position) const;
        void SetStopsIconsProperties(Circle& circle, Point position) const;
        void SetStopsTextCommonProperties(Text& text, const std::string& stopName, Point position) const;
        void SetStopsTextSubstrateProperties(Text& substrate, const std::string& stopName, Point position) const;
        void SetStopsTextTitleProperties(Text& title, const std::string& stopName, Point position) const;


    private:
        RenderSettings& renderSettings_;
    };


    template <typename PointInputIt>
    SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
    double max_width, double max_height, double padding)
    : padding_(padding) //
            {
                    // Если точки поверхности сферы не заданы, вычислять нечего
                    if (points_begin == points_end) {
                        return;
                    }

                    // Находим точки с минимальной и максимальной долготой
                    const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
                    min_lon_ = left_it->lng;
                    const double max_lon = right_it->lng;

                    // Находим точки с минимальной и максимальной широтой
                    const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
                    const double min_lat = bottom_it->lat;
                    max_lat_ = top_it->lat;

                    // Вычисляем коэффициент масштабирования вдоль координаты x
                    std::optional<double> width_zoom;
                    if (!IsZero(max_lon - min_lon_)) {
                        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
                    }

                    // Вычисляем коэффициент масштабирования вдоль координаты y
                    std::optional<double> height_zoom;
                    if (!IsZero(max_lat_ - min_lat)) {
                        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
                    }

                    if (width_zoom && height_zoom) {
                        // Коэффициенты масштабирования по ширине и высоте ненулевые,
                        // берём минимальный из них
                        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
                    } else if (width_zoom) {
                        // Коэффициент масштабирования по ширине ненулевой, используем его
                        zoom_coeff_ = *width_zoom;
                    } else if (height_zoom) {
                        // Коэффициент масштабирования по высоте ненулевой, используем его
                        zoom_coeff_ = *height_zoom;
                    }
            }


}
