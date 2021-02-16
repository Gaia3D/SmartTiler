## header only library
add_library(catch2 INTERFACE)

target_include_directories(catch2 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/catch
)

## header only library
add_library(spdlog INTERFACE)

add_dependencies(spdlog spdlog-install)

target_include_directories(spdlog INTERFACE
   ${CMAKE_CURRENT_LIST_DIR}/spdlog/include
)
