cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5)
project(Elastos.Service.CarrierGroup)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(ProjectDefaults)
include(AddSubModule)

set(ENABLE_TESTS ON CACHE BOOL "Build test cases")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(deps)
add_subdirectory(src)

install(TARGETS carrier.groupd
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

#if(ENABLE_TESTS)
#    add_subdirectory(tests)
#endif()
