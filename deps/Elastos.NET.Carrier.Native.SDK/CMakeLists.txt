project(Elastos.NET.Carrier.Native.SDK)

include(ProjectDefaults)
include(ExternalProject)

ExternalProject_Add(
    Elastos.NET.Carrier.Native.SDK

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/klx99/Elastos.NET.Carrier.Native.SDK/archive/bug_receipts.zip"
    URL_HASH SHA256=23df4f8e0090455c8c11e4086e98d1d7bcbe4a13b9040f3f045f3aed6f73d8d8
    DOWNLOAD_NAME "Elastos.NET.Carrier.Native.SDK-bug_receipts.zip"
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
