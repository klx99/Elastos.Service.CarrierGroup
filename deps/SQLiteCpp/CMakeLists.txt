project(SQLiteCpp)

include(ProjectDefaults)
include(ExternalProject)

ExternalProject_Add(
    SQLiteCpp

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/SRombauts/SQLiteCpp/archive/3.0.0.tar.gz"
    URL_HASH SHA256=152673de2d0deb918974be503dc624145c2c383e02eef9356fa67c08eb4c20f4
    DOWNLOAD_NAME "SQLiteCpp-3.0.0.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        -DENABLE_STATIC=TRUE
)

if(ENABLE_STATIC)
    export_static_library(SQLiteCpp)
endif()
