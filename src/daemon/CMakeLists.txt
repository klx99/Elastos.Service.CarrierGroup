add_executable(carrier.groupd)
target_sources(carrier.groupd PRIVATE
               "main.cpp")
target_include_directories(carrier.groupd PRIVATE
    )
target_include_directories(carrier.groupd PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(carrier.groupd PRIVATE
    common
    json)
