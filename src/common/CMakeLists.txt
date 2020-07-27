file( GLOB COMMON_SOURCES "*.cpp" )

if(${CMAKE_VERSION} VERSION_LESS "3.12")
    add_library(common STATIC ${COMMON_SOURCES})
else()
    add_library(common OBJECT ${COMMON_SOURCES})
endif()
target_include_directories(common PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${pkg-openssl_INCLUDE_DIRS}")

target_link_libraries(common PUBLIC
    )
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
    target_link_libraries(common PUBLIC
        log)
endif()


set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
