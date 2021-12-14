#include "bgfxRender.h"
#include "idlib/Str.h"
#include "FileSystem.h"
#include "CVarSystem.h"
#include "CmdSystem.h"


static PosColorVertex cube_vertices[] = {
    {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
    {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
    {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
    {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
};

static const uint16_t cube_tri_list[] = {
    0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6,
    1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7,
};
 //"shaders/v_simple.bin"
static bgfx::ShaderHandle createShader( const char * shaderFile, const char *name ) {
    int fSize = 0;
    const char *buffer = NULL;
    uInt sSize = 0;
    sSize = fileSystem->ReadFile( shaderFile, ( void ** ) &buffer );
    if ( sSize < 1 )
        common->FatalError( " Cant read shaderfile %s",shaderFile );
    sSize = ( uInt ) sSize;

    const bgfx::Memory *mem = bgfx::copy( buffer, sSize );
    const bgfx::ShaderHandle handle = bgfx::createShader( mem );
    bgfx::setName( handle, name );
    return handle;
}

void bgfxShutdown( bgfxContext_t *context ) { 
    bgfx::destroy( context->vbh );
    bgfx::destroy( context->ibh );
    bgfx::destroy( context->program );
    
    bgfx::shutdown( );
}

void bgfxInitShaders( bgfxContext_t *context ) {

    bgfx::VertexLayout pos_col_vert_layout;
    pos_col_vert_layout.begin( )
        .add( bgfx::Attrib::Position, 3, bgfx::AttribType::Float )
        .add( bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true )
        .end( );
    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(
        bgfx::makeRef( cube_vertices, sizeof( cube_vertices ) ),
        pos_col_vert_layout );
    bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(
        bgfx::makeRef( cube_tri_list, sizeof( cube_tri_list ) ) );


    bgfx::ShaderHandle vsh = createShader("shaders/v_simple.bin","vshader" );
    bgfx::ShaderHandle fsh = createShader( "shaders/f_simple.bin", "fsshader" );
    bgfx::ProgramHandle program = bgfx::createProgram( vsh, fsh, true );

    //context_t context;
    context->program = program;
    context->vbh = vbh;
    context->ibh = ibh;

    static bool cmdSystemSet = false;
    if (!cmdSystemSet) {

        cmdSystem->AddCommand( "CreateShader", []( const idCmdArgs &args ) 
            -> auto {
            createShader(args.Argv(1),args.Argv(2));
        }
        , CMD_FL_SYSTEM, "compiles the given shader file" );
    }   

}

void bgfxRender( bgfxContext_t *context ){
    float cam_rotation[16];
    bx::mtxRotateXYZ( cam_rotation, context->cam_pitch, context->cam_yaw, 0.0f );

    float cam_translation[16];
    bx::mtxTranslate( cam_translation, 0.0f, 0.0f, -5.0f );

    float cam_transform[16];
    bx::mtxMul( cam_transform, cam_translation, cam_rotation );

    float view[16];
    bx::mtxInverse( view, cam_transform );

    float proj[16];
    bx::mtxProj(
        proj, 60.0f, float( context->width ) / float( context->height ), 0.1f,
        100.0f, bgfx::getCaps( )->homogeneousDepth );

    bgfx::setViewTransform( 0, view, proj );

    float model[16];
    bx::mtxIdentity( model );
    bgfx::setTransform( model );

    bgfx::setVertexBuffer( 0, context->vbh );
    bgfx::setIndexBuffer( context->ibh );

    bgfx::submit( 0, context->program );
}