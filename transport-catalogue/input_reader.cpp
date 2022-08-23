#include "input_reader.h"


using transport_catalogue::detail::InQueries;
using transport_catalogue::TransportCatalogue;

void InQueries::readInput(std::istream& input) {
    int n;
    input >> n;
    std::string buffer;
    std::getline(input, buffer);
    while(n-- != 0){
        std::getline(input, buffer);
        size_t key_word_begin = 0;
        while(std::isspace(buffer[key_word_begin])){
            ++key_word_begin;
        }
        if (buffer.find("Bus", key_word_begin, 3) != std::string::npos) {
            buses.push_back(parseBus(buffer));
        } else if(buffer.find("Stop", key_word_begin, 4) != std::string::npos) {
            stops.push_back(parseStop(buffer));
        }
    }
}

void InQueries::executeQueries(TransportCatalogue& catalogue) {
    for(auto stop : stops) {
        catalogue.AddStop({std::move(stop.name_), {stop.latitude, stop.longitude}});
    }

    for(auto bus : buses) {
        std::vector<const Stop*> route;
        for(std::string& stopName : bus.stopNames) {
            route.push_back(&catalogue.GetStop(stopName));
        }
        catalogue.AddBus({std::move(bus.name_), std::move(route)});
    }

    for(auto stopFrom : stops) {
        const Stop* stopFromInCatalogue = &catalogue.GetStop(stopFrom.name_);
        for(auto &[stopTo, distance] : stopFrom.distance_to_stops) {
            const Stop* stopToInCatalogue = &catalogue.GetStop(stopTo);
            catalogue.SetStopsDistance(stopFromInCatalogue, stopToInCatalogue, distance);
        }
    }

    stops.clear();
    buses.clear();

}

InQueries::StopQuery InQueries::parseStop(std::string& stopQuery) {
    using namespace std::literals;
    std::vector<std::pair<std::string, int>> stops_distances;
    std::string_view str_view = stopQuery;
    size_t index = str_view.find("Stop"s) + 5;
    while (str_view[index] == ' ') {
        ++index;
    }
    str_view.remove_prefix(index);
    index = str_view.find(":"s);
    while (str_view[index - 1] == ' ') {
        --index;
    }
    std::string_view name = str_view.substr(0, index);

    str_view.remove_prefix(index);
    index = str_view.find(":"s);
    str_view.remove_prefix(index + 1);
    double latitude = std::stod(std::string(str_view));
    index = str_view.find(","s);
    str_view.remove_prefix(index + 1);
    double longitude = std::stod(std::string(str_view));

    index = str_view.find(","s);
    if(index != std::string::npos) {
        str_view.remove_prefix(index + 1);
        int distance = 0;
        while (true) {
            distance = std::stoi(std::string (str_view), &index);
            index = str_view.find("to "s);
            index += 3;
            str_view.remove_prefix(index);
            while(isspace(str_view[index])) {
                ++index;
            }
            index = str_view.find(","s);
            if(index == std::string::npos) {
                while(isspace(str_view[str_view.size() - 1])) {
                    str_view.remove_suffix(1);
                }
                std::string stop(str_view);
                stops_distances.push_back({std::move(stop), distance});
                break;
            } else{
                while(isspace(str_view[index - 1])){
                    --index;
                }
                std::string stop(str_view.substr(0, index));
                stops_distances.push_back({std::move(stop), distance});
                index = str_view.find(","s);
                str_view.remove_prefix(index + 1);
            }
        }
    }

    return {std::move(std::string(name)), latitude, longitude, std::move(stops_distances)};
}

InQueries::BusQuery InQueries::parseBus(std::string& busQuery) {
    BusQuery bus;
    bool busFl = false, nameFl = false;
    bool isRingRoute = (busQuery.find('>') != std::string::npos);
    std::string stop;
    for(char a : busQuery) {
        if(!busFl){
            if(a == ' ') {
                busFl = true;
            }
            continue;
        }
        if(!nameFl) {
            if(a == ':') {
                nameFl = true;
            } else {
                bus.name_.push_back(a);
            }
            continue;
        }
        if(stop.empty() && a == ' ') {
            continue;
        }
        if(a != '-' and a != '>') {
            stop.push_back(a);
        } else {
            stop.resize(stop.size() - 1);
            bus.stopNames.push_back(stop);
            stop.clear();
        }
    }
    if(!stop.empty()) {
        bus.stopNames.push_back(stop);
    }
    if(!isRingRoute) {
        for(int i = bus.stopNames.size() - 2; i >= 0; --i) {
            bus.stopNames.push_back(bus.stopNames[i]);
        }
    }
    return {std::move(bus.name_), std::move(bus.stopNames)};
}

void transport_catalogue::detail::tests::InQueriesToCatalogue(TransportCatalogue& catalogue) {
    InQueries a;
    std::stringstream queries;

    queries << "13\n"
            << "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\n"
            << "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino\n"
            << "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
            << "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka\n"
            << "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino\n"
            << "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\n"
            << "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam\n"
            << "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\n"
            << "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\n"
            << "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\n"
            << "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
            << "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
            << "Stop Prazhskaya: 55.611678, 37.603831\n";

    a.readInput(queries);
    a.executeQueries(catalogue);

    using namespace std::literals;

    {
        assert(catalogue.GetStop("Rasskazovka"s) == Stop("Rasskazovka"s, {55.632761, 37.333324}));
    }



    {
        BusInfo busInfo = catalogue.GetBusInfo("828"s);
        Bus bus = catalogue.GetBus("828"s);

        double geo_distance = ComputeDistance(bus.route_[0]->coordinates_, bus.route_[1]->coordinates_) +
                              ComputeDistance(bus.route_[1]->coordinates_, bus.route_[2]->coordinates_) +
                              ComputeDistance(bus.route_[2]->coordinates_, bus.route_[3]->coordinates_);
        double real_distance = catalogue.ComputeRealRouteDistance(bus);
        assert(busInfo == BusInfo("828"s, 4, 3, real_distance, real_distance/geo_distance));
    }

    {
        BusInfo busInfo = catalogue.GetBusInfo("750"s);
        Bus bus = catalogue.GetBus("750"s);

        double geo_distance = ComputeDistance(bus.route_[0]->coordinates_, bus.route_[1]->coordinates_) +
                              ComputeDistance(bus.route_[1]->coordinates_, bus.route_[2]->coordinates_) +
                              ComputeDistance(bus.route_[2]->coordinates_, bus.route_[3]->coordinates_) +
                              ComputeDistance(bus.route_[3]->coordinates_, bus.route_[4]->coordinates_) +
                              ComputeDistance(bus.route_[4]->coordinates_, bus.route_[5]->coordinates_) +
                              ComputeDistance(bus.route_[5]->coordinates_, bus.route_[6]->coordinates_)  ;
        double real_distance = catalogue.ComputeRealRouteDistance(bus);


        assert(busInfo == BusInfo("750"s, 7, 3, real_distance, real_distance/geo_distance));
    }


}