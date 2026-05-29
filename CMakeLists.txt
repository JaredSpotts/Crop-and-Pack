cmake_minimum_required(VERSION 3.20)
project(RenderHelper)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(nlohmann_json 3.12.0 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(render_helper
    main.cpp
    render_pass.cpp
)

target_link_libraries(render_helper PRIVATE
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
)