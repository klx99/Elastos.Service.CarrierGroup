file(GLOB CARRIER_SOURCES "*.hpp" "*.cpp")
if(${CMAKE_VERSION} VERSION_LESS "3.12")
    add_library(carrier STATIC ${CARRIER_SOURCES})
else()
    add_library(carrier OBJECT ${CARRIER_SOURCES})
endif()
target_include_directories(carrier PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(carrier PRIVATE
    common
    elacarrier)

add_dependencies(carrier
    Elastos.NET.Carrier.Native.SDK)
