cmake_minimum_required (VERSION 3.15)

project(bgfxTest)
set(RELEASE_NAME "gfxTest")

find_package(SDL2 REQUIRED)
find_package(BX REQUIRED)
find_package(BIMG REQUIRED)
find_package(BGFX REQUIRED)

include_directories(src)

add_executable(${RELEASE_NAME}
    src/main.cpp
)

source_group(src REGULAR_EXPRESSION src)


target_include_directories(
  ${RELEASE_NAME}
  PRIVATE "${CMAKE_SOURCE_DIR}/src"
  PRIVATE ${BX_INCLUDE_DIRS}
  PRIVATE ${BIMG_INCLUDE_DIRS}
  PRIVATE ${BGFX_INCLUDE_DIRS}
  PRIVATE ${SDL2_INCLUDE_DIRS}
  PRIVATE ${IMGUI_INCLUDE_DIRS}
)

target_link_libraries(
  ${RELEASE_NAME}
  ${BX_LIBRARIES}
  ${BIMG_LIBRARIES}
  ${BGFX_LIBRARIES}
  ${SDL2_LIBRARIES}
)

target_compile_options(
  ${RELEASE_NAME}
  PRIVATE /DBX_CONFIG_DEBUG=1
#  PRIVATE /DDIRECTINPUT_VERSION=0x0800
#  PRIVATE /DVERSION="${_DLL_VERSION}"
#  PRIVATE /D_CRT_SECURE_NO_WARNINGS
  PRIVATE /DNOMINMAX
  PRIVATE /std:c++17
)