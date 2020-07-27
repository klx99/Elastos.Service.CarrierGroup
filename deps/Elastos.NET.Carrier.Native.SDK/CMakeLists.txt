project(Elastos.NET.Carrier.Native.SDK)

include(ProjectDefaults)
include(ExternalProject)

ExternalProject_Add(
    Elastos.NET.Carrier.Native.SDK

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/elastos/Elastos.NET.Carrier.Native.SDK/archive/release-v5.6.2.zip"
    URL_HASH SHA256=b15635b64a8d55ed8af8a3c10c6ef64faaeec9d3a61d9d0e98960d94197864a9
    DOWNLOAD_NAME "Elastos.NET.Carrier.Native.SDK-v5.6.2.zip"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1
    LOG_DOWNLOAD FALSE

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PROJECT_INT_DIST_DIR}
        -DENABLE_SHARED=TRUE
        -DENABLE_STATIC=FALSE
        -DENABLE_APPS=FALSE
        -DENABLE_TESTS=FALSE
        -DENABLE_DOCS=FALSE
)

export_shared_library(elacarrier)
add_library(libelacarrier-shared INTERFACE)
target_link_libraries(libelacarrier-shared INTERFACE elacarrier libcrystal-shared)

export_shared_library(crystal)
add_library(libcrystal-shared INTERFACE)
target_link_libraries(libcrystal-shared INTERFACE crystal)
