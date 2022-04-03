cmake_minimum_required (VERSION 3.15)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

project(bgfxTest)
set(RELEASE_NAME "gfxTest")

find_package(SDL2 REQUIRED)
find_package(BX REQUIRED)
find_package(BIMG REQUIRED)
find_package(BGFX REQUIRED)
find_package(IMGUI REQUIRED)
find_package(imguizmo REQUIRED)
find_package(ZLIB REQUIRED)
find_package(freetype CONFIG REQUIRED)

add_definitions(-DID_SIZEOFPTR=${CMAKE_SIZEOF_VOID_P})
add_compile_definitions(IMGUI_USER_CONFIG="${PROJECT_SOURCE_DIR}/src/bgfx-imgui/imconfig.h")

function(add_globbed_headers SRCLIST PATHPREFIX)
	file(GLOB_RECURSE tmp_hdrs RELATIVE "${CMAKE_SOURCE_DIR}/idFramework/" "${PATHPREFIX}/*.h")
	set(${SRCLIST} ${tmp_hdrs} ${${SRCLIST}} PARENT_SCOPE)
endfunction()

set(src_idSWF
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Bitstream.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Bitstream.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Dictionary.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Enums.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Events.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Image.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Load.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Names.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_ParmList.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_ParmList.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_PlaceObject.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Render.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_ScriptFunction.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_ScriptFunction.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_ScriptObject.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_ScriptObject.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_ScriptVar.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_ScriptVar.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_ShapeParser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_ShapeParser.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Shapes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Sounds.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_SpriteInstance.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_SpriteInstance.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Sprites.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Sprites.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Text.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_TextInstance.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_TextInstance.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Types.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Zlib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Interpreter.cpp
)

set(src_idFramework
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/sys/platform.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/File.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/Common.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/FileSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/CmdSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/CVarSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/minizip/ioapi.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/EventLoop.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/KeyInput.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/UsercmdGen.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/Compressor.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/console.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idImGui/idImConsole.h

	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/Font.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/File.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/FileSystem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/CmdSystem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/CVarSystem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/minizip/ioapi.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/minizip/unzip.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/EventLoop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/KeyInput.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/UsercmdGen.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/Compressor.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/sys/cpu.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/sys/threads.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/sys/events.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/sys/sys_local.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/sys/win32/win_local.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/sys/win32/win_input.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/sys/win32/win_main.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/sys/win32/win_net.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/sys/win32/win_shared.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/sys/win32/win_syscon.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idImGui/idImConsole.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/stubs/common_stubs.cpp
	)

#add_globbed_headers(src_idFramework "idFramework")
#
#
set(src_idlib
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/bv/Bounds.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/bv/Frustum.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/bv/Sphere.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/bv/Box.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/geometry/DrawVert.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/geometry/Winding2D.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/geometry/Surface_SweptSpline.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/geometry/Winding.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/geometry/Surface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/geometry/Surface_Patch.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/geometry/TraceModel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/geometry/JointTransform.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/hashing/CRC32.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/hashing/MD4.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/hashing/MD5.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Angles.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Lcp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Math.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Matrix.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Ode.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Plane.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Pluecker.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Polynomial.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Quat.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Rotation.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Simd.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Simd_Generic.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Simd_AltiVec.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Simd_MMX.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Simd_3DNow.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Simd_SSE.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Simd_SSE2.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Simd_SSE3.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/math/Vector.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/BitMsg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/LangDict.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/Lexer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/Lib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/containers/HashIndex.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/Dict.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/Str.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/Parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/RectAllocator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/MapFile.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/CmdArgs.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/Token.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/Base64.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/Timer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/idFramework/idlib/Heap.cpp
)

add_globbed_headers(src_idlib "idlib")

add_library(idFramework STATIC ${src_idlib})

set(Sources 
	${PROJECT_SOURCE_DIR}/src/bgfx-stubs/font/utf8.cpp
	${PROJECT_SOURCE_DIR}/src/bgfx-stubs/font/text_buffer_manager.cpp
	${PROJECT_SOURCE_DIR}/src/bgfx-stubs/cube_atlas.cpp
	${PROJECT_SOURCE_DIR}/src/bgfx-stubs/bgfxRenderer.cpp
	${PROJECT_SOURCE_DIR}/src/bgfx-stubs/Renderers/ForwardRenderer.cpp
	${PROJECT_SOURCE_DIR}/src/bgfx-stubs/Renderers/LightShader.cpp
	${PROJECT_SOURCE_DIR}/src/bgfx-stubs/Renderers/PBRShader.cpp
	${PROJECT_SOURCE_DIR}/src/bgfx-stubs/bgfxImage.cpp
	${PROJECT_SOURCE_DIR}/src/bgfx-stubs/bgfxRender.cpp
	${PROJECT_SOURCE_DIR}/src/bgfx-stubs/bgfxCallback.cpp
	${PROJECT_SOURCE_DIR}/src/gltf-edit/TextEditor.cpp
    ${PROJECT_SOURCE_DIR}/src/gltf-edit/TextEditor.h
    ${PROJECT_SOURCE_DIR}/src/gltf-edit/gltfEditor.h
	${PROJECT_SOURCE_DIR}/src/gltf-edit/gltfEditor.cpp
	${PROJECT_SOURCE_DIR}/src/gltf-edit/gltfParser.cpp
    ${PROJECT_SOURCE_DIR}/src/bgfx-imgui/imgui_impl_bgfx.cpp
    ${PROJECT_SOURCE_DIR}/src/bgfx-imgui/imgui_impl_bgfx.h
    ${PROJECT_SOURCE_DIR}/src/sdl-imgui/imgui_impl_sdl.h
    ${PROJECT_SOURCE_DIR}/src/sdl-imgui/imgui_impl_sdl.cpp
    ${PROJECT_SOURCE_DIR}/src/bgfx-imgui/imconfig.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/swf/SWF_Abc.cpp
	${PROJECT_SOURCE_DIR}/src/main.cpp)

include_directories(src)
include_directories(src/idFramework)
include_directories(${ZLIB_INCLUDE_DIRS})
add_executable(${RELEASE_NAME} ${Sources} ${src_idFramework} ${src_idSWF} )
source_group(src REGULAR_EXPRESSION src)

target_include_directories(
  ${RELEASE_NAME}
  PRIVATE "${CMAKE_SOURCE_DIR}/src"
  private "${CMAKE_SOURCE_DIR}/src/idFramework"
  PRIVATE ${BX_INCLUDE_DIRS}
  PRIVATE ${BIMG_INCLUDE_DIRS}
  PRIVATE ${BGFX_INCLUDE_DIRS}
  PRIVATE ${SDL2_INCLUDE_DIRS}
  PRIVATE ${IMGUI_INCLUDE_DIRS}
  private ${ZLIB_INCLUDE_DIRS}
  private ${FREETYPE_INCLUDE_DIRS}
  
)
#ghp_czo78ObdlRrDLZ7QFdj9wOO0cbrgnK5GL0x7

target_link_libraries(
  ${RELEASE_NAME}
  ${BX_LIBRARIES}
  ${BIMG_LIBRARIES}
  ${BGFX_LIBRARIES}
  ${SDL2_LIBRARIES}
  imgui::imgui
  imguizmo::imguizmo
  idFramework
  ${ZLIB_LIBRARY}
  freetype
)

target_compile_options(
  ${RELEASE_NAME}
  PRIVATE /DBX_CONFIG_DEBUG=1
#  PRIVATE /DDIRECTINPUT_VERSION=0x0800
#  PRIVATE /DVERSION="${_DLL_VERSION}"
#  PRIVATE /D_CRT_SECURE_NO_WARNINGS
  PRIVATE /DNOMINMAX
  PRIVATE /std:c++latest
  PRIVATE /wd4068 # unknown pragma
  /DIDSTR_NO_REDIRECT
  PUBLIC /Zc:__cplusplus
  PRIVATE /D_cpp
)