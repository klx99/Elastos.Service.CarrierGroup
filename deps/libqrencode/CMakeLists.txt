project(libqrencode)

include(ProjectDefaults)
include(ExternalProject)

ExternalProject_Add(
    libqrencode

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://fukuchi.org/works/qrencode/qrencode-4.0.2.tar.gz"
    URL_HASH SHA256=dbabe79c07614625d1f74d8c0ae2ee5358c4e27eab8fd8fe31f9365f821a3b1d
    DOWNLOAD_NAME "qrencode-4.0.2.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1
    LOG_DOWNLOAD FALSE

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        -DWITH_TOOLS=NO
        "-DCMAKE_PREFIX_PATH=${PROJECT_INT_DIST_DIR}"
)

add_library(libqrencode-static INTERFACE)
target_link_libraries(libqrencode-static INTERFACE qrencode png z)
