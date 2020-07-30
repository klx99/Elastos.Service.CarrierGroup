file(GLOB SQLDB_SOURCES "*.hpp" "*.cpp")
if(${CMAKE_VERSION} VERSION_LESS "3.12")
    add_library(sqldb STATIC ${SQLDB_SOURCES})
else()
    add_library(sqldb OBJECT ${SQLDB_SOURCES})
endif()
target_include_directories(sqldb PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(sqldb PRIVATE
    sqlite3
    libSQLiteCpp.a)

add_dependencies(sqldb SQLiteCpp)