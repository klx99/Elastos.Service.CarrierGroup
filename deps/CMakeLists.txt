include(AddSubModule)

add_submodule(platform-specific)
# add_submodule(libconfig
#     DEPENDS platform-specific)
add_submodule(Elastos.NET.Carrier.Native.SDK
    DEPENDS platform-specific)
add_submodule(SQLiteCpp
    DEPENDS platform-specific)
# add_submodule(sqlite
#     DEPENDS platform-specific)
# add_submodule(CUnit
#     DEPENDS platform-specific)
# add_submodule(zlib
#     DEPENDS platform-specific)
# add_submodule(libpng
#     DEPENDS zlib platform-specific)
# add_submodule(libqrencode
#     DEPENDS libpng platform-specific)


