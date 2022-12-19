#include "map_renderer.h"


namespace renderer {

    Point SphereProjector::operator()(Coordinates coords) const {
        return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    MapRenderer::MapRenderer(RenderSettings &renderSettings) :
            renderSettings_(renderSettings) {
    }

    SphereProjector MapRenderer::GetSphereProjector(const std::vector<Coordinates> &allPoint) const {
        return SphereProjector(allPoint.begin(),
                               allPoint.end(),
                               renderSettings_.width_,
                               renderSettings_.height_,
                               renderSettings_.padding_);
    }

    Color MapRenderer::GetStrokeColorByOrder(int number) const {
        int colorCount = renderSettings_.color_palette_.size();
        int index = number % colorCount;
        if (number <= colorCount) {
            index = number - 1;
        } else if (index == 0) {
            index = colorCount - 1;
        } else {
            index = index - 1;
        }
        return renderSettings_.color_palette_[index];
    }

    const RenderSettings& MapRenderer::GetRenderSettings() const {
        return renderSettings_;
    }

    void MapRenderer::SetLineProperties(Polyline &line, int number) const {
        using namespace std::literals;
        line.SetStrokeColor(GetStrokeColorByOrder(number));
        line.SetFillColor("none"s);
        line.SetStrokeWidth(renderSettings_.line_width_);
        line.SetStrokeLineCap(StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(StrokeLineJoin::ROUND);
    }

    void MapRenderer::SetBusTextCommonProperties(Text &text, const std::string &data, Point position) const {
        using namespace std::literals;
        text.SetPosition(position);
        text.SetOffset({renderSettings_.bus_label_offset_.first,
                        renderSettings_.bus_label_offset_.second});
        text.SetFontSize(renderSettings_.bus_label_font_size_);
        text.SetFontFamily("Verdana"s);
        text.SetFontWeight("bold"s);
        text.SetData(data);
    }

    void
    MapRenderer::SetBusTextSubstrateProperties(Text &text, const std::string &data, Point position) const {
        SetBusTextCommonProperties(text, data, position);
        text.SetFillColor(renderSettings_.underlayer_color_);
        text.SetStrokeColor(renderSettings_.underlayer_color_);
        text.SetStrokeWidth(renderSettings_.underlayer_width_);
        text.SetStrokeLineJoin(StrokeLineJoin::ROUND);
        text.SetStrokeLineCap(StrokeLineCap::ROUND);
    }

    void MapRenderer::SetBusTextTitleProperties(Text &text, const std::string &data, const Color &color,
                                                Point position) const {
        SetBusTextCommonProperties(text, data, position);
        text.SetFillColor(color);
    }

    void MapRenderer::SetStopsIconsProperties(Circle &circle, Point position) const {
        using namespace std::literals;
        circle.SetCenter(position);
        circle.SetRadius(renderSettings_.stop_radius_);
        circle.SetFillColor("white"s);
    }

    void
    MapRenderer::SetStopsTextCommonProperties(Text &text, const std::string &stopName, Point position) const {
        using namespace std::literals;
        text.SetPosition(position);
        text.SetOffset({renderSettings_.stop_label_offset_.first,
                        renderSettings_.stop_label_offset_.second});
        text.SetFontSize(renderSettings_.stop_label_font_size_);
        text.SetFontFamily("Verdana"s);
        text.SetData(stopName);
    }

    void MapRenderer::SetStopsTextSubstrateProperties(Text &substrate, const std::string &stopName,
                                                      Point position) const {
        using namespace std::literals;
        SetStopsTextCommonProperties(substrate, stopName, position);
        substrate.SetFillColor(renderSettings_.underlayer_color_);
        substrate.SetStrokeColor(renderSettings_.underlayer_color_);
        substrate.SetStrokeWidth(renderSettings_.underlayer_width_);
        substrate.SetStrokeLineJoin(StrokeLineJoin::ROUND);
        substrate.SetStrokeLineCap(StrokeLineCap::ROUND);
    }

    void MapRenderer::SetStopsTextTitleProperties(Text &title, const std::string &stopName, Point position) const {
        using namespace std::literals;
        SetStopsTextCommonProperties(title, stopName, position);
        title.SetFillColor("black"s);
    }

}