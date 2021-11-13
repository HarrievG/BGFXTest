#include "SDL.h"
#include "SDL_syswm.h"
#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bx/math.h"
#include "imgui.h"
#include "sdl-imgui/imgui_impl_sdl.h"

int main( int argc, char **argv )
{
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize. SDL_Error: %s\n", SDL_GetError( ) );
        return 1;
    }


    const int width = 800;
    const int height = 600;
    SDL_Window *window = SDL_CreateWindow(
        argv[0], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width,
        height, SDL_WINDOW_SHOWN );

    if ( window == nullptr ) {
        printf( "Window could not be created. SDL_Error: %s\n", SDL_GetError( ) );
        return 1;
    }
 
#if !BX_PLATFORM_EMSCRIPTEN
    SDL_SysWMinfo wmi;
    SDL_VERSION( &wmi.version );
    if ( !SDL_GetWindowWMInfo( window, &wmi ) ) {
        printf(
            "SDL_SysWMinfo could not be retrieved. SDL_Error: %s\n",
            SDL_GetError( ) );
        return 1;
    }
    bgfx::renderFrame( ); // single threaded mode
#endif // !BX_PLATFORM_EMSCRIPTEN
    bgfx::init();
    return 0;
}