file(GLOB DAEMON_SOURCES "*.hpp" "*.cpp")
add_executable(carrier.groupd)
target_sources(carrier.groupd PRIVATE
               ${DAEMON_SOURCES})
target_include_directories(carrier.groupd PRIVATE
    )
target_include_directories(carrier.groupd PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(carrier.groupd PRIVATE
    carrier
    common
    json
    sqldb)
