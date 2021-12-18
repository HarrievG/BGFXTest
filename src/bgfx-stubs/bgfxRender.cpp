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

    context->colorUniformHandle = bgfx::createUniform( "colorUniformHandle", bgfx::UniformType::Sampler );
    context->fbh.idx = bgfx::kInvalidHandle;
    context->Bgra8 = 0;
    if ( ( BGFX_CAPS_TEXTURE_BLIT | BGFX_CAPS_TEXTURE_READ_BACK ) == ( bgfx::getCaps( )->supported & ( BGFX_CAPS_TEXTURE_BLIT | BGFX_CAPS_TEXTURE_READ_BACK ) ) ) {
        context->rb = bgfx::createTexture2D( context->width, context->height, false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST );
    }

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

    if ( !bgfx::isValid( context->fbh ))
/*        || m_oldWidth != m_width
        || m_oldHeight != m_height
        || m_oldReset != m_reset )*/ {
        // Recreate variable size render targets when resolution changes.
        //m_oldWidth = m_width;
        //m_oldHeight = m_height;
        //m_oldReset = m_reset;

        if ( bgfx::isValid( context->fbh ) ) {
            bgfx::destroy( context->fbh );
        }

        context->fbTextureHandle[0] = bgfx::createTexture2D(
            uint16_t( context->width )
            , uint16_t( context->height )
            , false
            , 1
            , bgfx::TextureFormat::BGRA8
            , BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
        );

        const uint64_t textureFlags = BGFX_TEXTURE_RT_WRITE_ONLY | BGFX_TEXTURE_RT ;

        bgfx::TextureFormat::Enum depthFormat =
            bgfx::isTextureValid( 0, false, 1, bgfx::TextureFormat::D16, textureFlags ) ? bgfx::TextureFormat::D16
            : bgfx::isTextureValid( 0, false, 1, bgfx::TextureFormat::D24S8, textureFlags ) ? bgfx::TextureFormat::D24S8
            : bgfx::TextureFormat::D32
            ;

        context->fbTextureHandle[1] = bgfx::createTexture2D(
            uint16_t( context->width )
            , uint16_t( context->height )
            , false
            , 1
            , depthFormat
            , textureFlags
        );

        context->fbh = bgfx::createFrameBuffer( BX_COUNTOF( context->fbTextureHandle), context->fbTextureHandle, true );
        bgfx::ViewId rttView = 1;
        bgfx::setViewName( rttView, "backBuffer" );
        //bgfx::setViewClear( rttView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xff0000ff, 0, 0 );
        bgfx::setViewRect( rttView, 0, 0, bgfx::BackbufferRatio::Equal );
        bgfx::setViewFrameBuffer( rttView, context->fbh );
    }

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
    bgfx::setViewTransform( 1, view, proj );

    float modelTransform[16];
    float modelRotation[16];
    float modelTranslation[16];
    float modelScale[16];
    float tmp[16];
    bx::mtxIdentity( modelTranslation );
    bx::mtxRotateXYZ( modelRotation, abs( ( 0.001f * com_frameTime ) ), abs( ( 0.001f * com_frameTime ) ), 0.0f );
    bx::mtxScale( modelScale, 0.8f + idMath::ClampFloat( 0.2f, 10.0f, ( abs( sin( 0.001f * com_frameTime ) ) ) ) );// sin( com_frameTime ) );

    bx::mtxMul( tmp, modelScale, modelRotation );
    bx::mtxMul( modelTransform, tmp, modelTranslation );
    bx::mtxIdentity( tmp);

    bgfx::setTransform( tmp );
    
    bgfx::setVertexBuffer( 0, context->vbh );
    bgfx::setIndexBuffer( context->ibh );
    bgfx::submit( 0, context->program );

    bgfx::setTransform( modelTransform );
    bgfx::setVertexBuffer( 0, context->vbh );
    bgfx::setIndexBuffer( context->ibh );
    bgfx::submit( 1, context->program );
    

    if ( bgfx::isValid( context->rb ) )
        bgfx::blit( 2, context->rb, 0, 0, context->fbTextureHandle[0] );
}

void bgfxAdd( bgfxRenderable *renderable ) { }
