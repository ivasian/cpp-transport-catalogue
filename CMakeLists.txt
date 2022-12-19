cmake_minimum_required(VERSION 3.10)

project(14_5_1_1 CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH /snap/protobuf/current)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto)
set(14_5_1_1_FILES main.cpp domain.cpp domain.h geo.cpp geo.h graph.h json.cpp json.h json_builder.cpp json_builder.h json_reader.cpp json_reader.h  map_renderer.cpp map_renderer.h ranges.h request_handler.cpp request_handler.h router.h svg.cpp svg.h transport_catalogue.cpp transport_catalogue.h transport_router.cpp transport_router.h serialization.h serialization.cpp)

add_executable(14_5_1_1 ${PROTO_SRCS} ${PROTO_HDRS} ${14_5_1_1_FILES} cmake-build-debug/transport_catalogue.pb.cc cmake-build-debug/transport_catalogue.pb.h)
target_include_directories(14_5_1_1 PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(14_5_1_1 PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(14_5_1_1 "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
