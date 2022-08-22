#include "stat_reader.h"

using transport_catalogue::detail::OutQueries;

void OutQueries::readInput(){
    int n;
    in_ >> n;
    std::string buffer;
    std::getline(in_, buffer);
    while(n-- != 0){
        std::getline(in_, buffer);
        size_t key_word_begin = 0;
        while(std::isspace(buffer[key_word_begin])){
            ++key_word_begin;
        }
        if(buffer.find("Bus", key_word_begin, 3) != std::string::npos){
            executeBusInfoQuery(parseBusInfoQuery(buffer));
        } else if(buffer.find("Stop", key_word_begin, 4) != std::string::npos){
            executeStopInfoQuery(parseStopInfoQuery(buffer));
        }
    }
}

OutQueries::BusInfoQuery OutQueries::parseBusInfoQuery(std::string& busInfoQuery){
    bool busKeyWord = true;
    std::string name;
    for(char a : busInfoQuery){
        if(busKeyWord){
            if(a == ' '){
                busKeyWord = false;
            }
            continue;
        }
        if(!(name.empty() && a == ' ')){
            name.push_back(a);
        }
    }
    return {std::move(name)};
}

OutQueries::StopInfoQuery OutQueries::parseStopInfoQuery(std::string& stopInfoQuery){
    bool busKeyWord = true;
    std::string name;
    for(char a : stopInfoQuery){
        if(busKeyWord){
            if(a == ' '){
                busKeyWord = false;
            }
            continue;
        }
        if(!(name.empty() && a == ' ')){
            name.push_back(a);
        }
    }
    return {std::move(name)};
}

const std::vector<OutQueries::BusInfoQuery>& OutQueries::GetBusInfoQueries() const{
    return OutQueries::busInfoQueries_;
}


void OutQueries::executeBusInfoQuery(BusInfoQuery query){
    using namespace std::literals;
    out_ << std::setprecision(6);
    try{
        auto busInfo = transportCatalogue_.GetBusInfo(query.name_);
        out_ << "Bus "s << busInfo.name_ << ": "s << busInfo.stops_amount_
            << " stops on route, "s << busInfo.unique_stops_amount_
            << " unique stops, "s << busInfo.route_length_ << " route length, "
            << busInfo.curvature_ << " curvature"s <<std::endl;
    } catch (...){
        out_ << "Bus "s << query.name_ << ": not found" << std::endl;
    }
}

void OutQueries::executeStopInfoQuery(StopInfoQuery query){
    using namespace std::literals;
    out_ << std::setprecision(6);
    try{
        const std::set<std::string_view> buses = transportCatalogue_.GetStopInfo(query.name_);
        if(buses.empty()){
            out_ << "Stop "s << query.name_ << ": no buses"s;
        } else {
            out_ << "Stop "s << query.name_ << ": buses "s;
            bool isfirst = true;
            for(auto bus : buses){
                if(!isfirst){
                    out_ << " "s;
                }
                isfirst = false;
                out_ << bus;
            }
        }
        out_ << std::endl;
    } catch (...){
        out_ << "Stop "s << query.name_ << ": not found" << std::endl;
    }
}

void transport_catalogue::detail::tests::OutQueriesFromCatalogue(transport_catalogue::TransportCatalogue& catalogue){
    using namespace std::literals;
    catalogue.AddStop({"Tolstopaltsevo"s, {55.611087, 37.208290}});
    catalogue.AddStop({"Marushkino"s, {55.595884, 37.209755}});
    catalogue.AddStop({"Rasskazovka"s, {55.632761, 37.333324}});
    catalogue.AddStop({"Biryulyovo Zapadnoye"s, {55.574371, 37.651700}});
    catalogue.AddStop({"Biryusinka"s, {55.581065, 37.648390}});

    catalogue.AddStop({"Biryusinka"s, {55.581065, 37.648390}});
    catalogue.AddStop({"Universam"s, {55.587655, 37.645687}});
    catalogue.AddStop({"Biryulyovo Tovarnaya"s, {55.592028, 37.653656}});
    catalogue.AddStop({"Biryulyovo Oshibka"s, {55.592011, 37.653226}});
    catalogue.AddStop({"Biryulyovo Passazhirskaya"s, {55.580999, 37.659164}});


    catalogue.AddBus({"256"s, {&catalogue.GetStop("Biryulyovo Zapadnoye"s),
                                   &catalogue.GetStop("Biryusinka"s),
                                   &catalogue.GetStop("Universam"s),
                                   &catalogue.GetStop("Biryulyovo Tovarnaya"s),
                                   &catalogue.GetStop("Biryulyovo Passazhirskaya"s),
                                   &catalogue.GetStop("Biryulyovo Zapadnoye"s)}});

    catalogue.AddBus({"750"s, {&catalogue.GetStop("Tolstopaltsevo"s),
                               &catalogue.GetStop("Marushkino"s),
                               &catalogue.GetStop("Rasskazovka"s),
                               &catalogue.GetStop("Marushkino"s),
                               &catalogue.GetStop("Tolstopaltsevo"s),
                               &catalogue.GetStop("Biryulyovo Zapadnoye"s)}});


    std::stringstream queries, answers;
    queries << "4\n"
            << "Bus 256\n"
            << "Bus 750\n"
            << "Bus 751\n"
            << "Stop 222\n";

    OutQueries outQueries(catalogue, queries, std::cout);
    outQueries.readInput();
}