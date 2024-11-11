# Configure with [fxsdk build-fx] or [fxsdk build-cg], which provide the
# toolchain file and module path of the fxSDK

cmake_minimum_required(VERSION 3.15)
project(vimsgint)

include(GenerateG1A)
include(GenerateG3A)
include(GenerateHH2Bin)
include(Fxconv)
find_package(Gint 2.9 REQUIRED)

set(SOURCES
  ../VIMS3D.c
  ../game.c
  ../worldobj.c
  ../world.c
  ../snprintf_light.c
  ../texturemap.c
  ../mesh.c
  ../raster.c
  ../VIMS_defs.c
  ../vec.c
  ../fixed.c
  ../hires_timer.c
  # ...
)
# Shared assets, fx-9860G-only assets and fx-CG-50-only assets
set(ASSETS
  # ...
)
set(ASSETS_fx
  # ...
)
set(ASSETS_cg
  # ...
)

fxconv_declare_assets(${ASSETS} ${ASSETS_fx} ${ASSETS_cg} WITH_METADATA)

add_executable(vimsgint ${SOURCES} ${ASSETS} ${ASSETS_${FXSDK_PLATFORM}})
target_compile_options(vimsgint PRIVATE -Wall -Wextra -Wno-unknown-pragmas -DGINT_PORT -O3 -g -flto)
target_link_libraries(vimsgint Gint::Gint)

if("${FXSDK_PLATFORM_LONG}" STREQUAL fx9860G)
  generate_g1a(TARGET vimsgint OUTPUT "VIMSGINT.g1a"
    NAME "VIMSGINT" ICON assets-fx/icon.png)
elseif("${FXSDK_PLATFORM_LONG}" STREQUAL fxCG50)
  generate_g3a(TARGET vimsgint OUTPUT "VIMSGINT.g3a"
    NAME "VIMSGINT" ICONS assets-cg/icon-uns.png assets-cg/icon-sel.png)
elseif("${FXSDK_PLATFORM_LONG}" STREQUAL fx9860G_G3A)
  generate_g3a(TARGET vimsgint OUTPUT "VIMSGINT-fx.g3a"
    NAME "VIMSGINT-fx" ICONS assets-cg/icon-uns.png assets-cg/icon-sel.png)
elseif("${FXSDK_PLATFORM_LONG}" STREQUAL fxCP)
  generate_hh2_bin(TARGET vimsgint OUTPUT "VIMSGINT-hh2.bin")
endif()
