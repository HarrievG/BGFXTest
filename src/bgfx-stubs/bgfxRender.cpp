#include "bgfxRender.h"
#include "idlib/Str.h"
#include "FileSystem.h"
#include "CVarSystem.h"
#include "CmdSystem.h"
#include <ImGuizmo.h>
#include "bgfxImage.h"

static PosColorVertex cube_vertices[] = {
    {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
    {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
    {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
    {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
};

static const uint16_t cube_tri_list[] = {
    0, 2 , 1, 1, 2, 3, 4, 5, 6, 5, 7, 6, 0, 4, 2, 4, 6, 2,
    1, 3, 5, 5, 3, 7, 0, 1, 4, 4, 1, 5, 2, 6, 3, 6, 7, 3,
};

idCVar r_customWidth( "r_customWidth", "1920", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, "custom screen width. set r_mode to -1 to activate" );
idCVar r_customHeight( "r_customHeight", "1080", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, "custom screen height. set r_mode to -1 to activate" );
idCVar r_aspectRatio( "r_aspectRatio", "-1", CVAR_RENDERER | CVAR_INTEGER | CVAR_ARCHIVE, "aspect ratio of view:\n0 = 4:3\n1 = 16:9\n2 = 16:10\n-1 = auto (guess from resolution)", -1, 2 );
idCVar r_mode( "r_mode", "-1", CVAR_ARCHIVE | CVAR_RENDERER | CVAR_INTEGER, "video mode number" );

idList<bgfxCallback> bgfxCallbackList;

xthreadInfo RenderThread;
bool	    RenderThreadRunning;

bgfx::ShaderHandle bgfxCreateShader( const char * shaderFile, const char *name ) {
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
    
    bgfxCallbackList.Clear( );

    bgfx::shutdown( );
}
void bgfxCreatePbrContext(bgfxPbrContext_t & context )
{
    //create shaders
    bgfx::ShaderHandle vsh = bgfxCreateShader( "shaders/v_pbr.bin", "pbr_vshader" );
    bgfx::ShaderHandle fsh = bgfxCreateShader( "shaders/f_pbr.bin", "pbr_fsshader" );
    context.pbrProgram = bgfx::createProgram( vsh, fsh, true );
    //create Uniforms
    context.s_baseColor         = bgfx::createUniform( "s_baseColor", bgfx::UniformType::Sampler );
    context.s_normal            = bgfx::createUniform( "s_normal", bgfx::UniformType::Sampler );
    context.s_metallicRoughness = bgfx::createUniform( "s_metallicRoughness", bgfx::UniformType::Sampler );
    context.s_emissive          = bgfx::createUniform( "s_emissive", bgfx::UniformType::Sampler );
    context.s_occlusion         = bgfx::createUniform( "s_occlusion", bgfx::UniformType::Sampler );

    // We are going to pack our baseColorFactor, emissiveFactor, roughnessFactor
    // and metallicFactor into this uniform
    context.u_factors           = bgfx::createUniform( "u_factors", bgfx::UniformType::Vec4, 3 );
    context.u_cameraPos         = bgfx::createUniform( "u_cameraPos", bgfx::UniformType::Vec4 );
    context.u_normalTransform   = bgfx::createUniform( "u_normalTransform", bgfx::UniformType::Mat4 );
}

void bgfxInitShaders( bgfxContext_t *context ) {

    bgfxCreatePbrContext(context->pbrContext);
    bgfxStartImageLoadThread();
    bgfx::VertexLayout pos_col_vert_layout;
    pos_col_vert_layout.begin( )
        .add( bgfx::Attrib::Position, 3, bgfx::AttribType::Float )
        .add( bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8,true )
        .end( );
    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(
        bgfx::makeRef( cube_vertices, sizeof( cube_vertices ) ),
        pos_col_vert_layout );
    bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(
        bgfx::makeRef( cube_tri_list, sizeof( cube_tri_list ) ) );

    bgfx::ShaderHandle vsh      = bgfxCreateShader("shaders/v_simple.bin","vshader" );
    bgfx::ShaderHandle fsh      = bgfxCreateShader( "shaders/f_simple.bin", "fsshader" );
    bgfx::ProgramHandle program = bgfx::createProgram( vsh, fsh, true );
    
    bgfxCreatePbrContext( context->pbrContext );
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

	bx::mtxProj(
		context->cameraProjection.ToFloatPtr(), 30.0, float( context->width ) / float( context->height ), 0.1f,
		10000.0f, bgfx::getCaps( )->homogeneousDepth, bx::Handness::Right );

	bx::mtxIdentity(context->cameraView.ToFloatPtr( ));


    static bool cmdSystemSet = false;
    if (!cmdSystemSet) {

        cmdSystem->AddCommand( "CreateShader", []( const idCmdArgs &args ) 
            -> auto {
			bgfxCreateShader(args.Argv(1),args.Argv(2));
        }
        , CMD_FL_SYSTEM, "compiles the given shader file" );
		cmdSystemSet = true;
    }   

}

void bgfxCreateMrtTarget(bgfxMrtContext_t & context,const char * name)
{
	if ( ( BGFX_CAPS_TEXTURE_BLIT | BGFX_CAPS_TEXTURE_READ_BACK ) == ( bgfx::getCaps( )->supported & ( BGFX_CAPS_TEXTURE_BLIT | BGFX_CAPS_TEXTURE_READ_BACK ) ) ) {
		context.rb = bgfx::createTexture2D( context.width, context.height, false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_BLIT_DST );
	}

	if ( bgfx::isValid( context.fbh ) ) {
		common->DWarning("Destroying framebuffer %i",context.fbh.idx );
		bgfx::destroy( context.fbh );
	}else
	{
		context.fbTextureHandles[0] = bgfx::createTexture2D(
			uint16_t( context.width )
			, uint16_t( context.height )
			, false
			, 1
			, bgfx::TextureFormat::BGRA8
			, BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
		);

		const uint64_t textureFlags = BGFX_TEXTURE_RT_WRITE_ONLY | BGFX_TEXTURE_RT;

		bgfx::TextureFormat::Enum depthFormat =
			bgfx::isTextureValid( 0, false, 1, bgfx::TextureFormat::D16, textureFlags ) ? bgfx::TextureFormat::D16
			: bgfx::isTextureValid( 0, false, 1, bgfx::TextureFormat::D24S8, textureFlags ) ? bgfx::TextureFormat::D24S8
			: bgfx::TextureFormat::D32;

		context.fbTextureHandles[1] = bgfx::createTexture2D(
			uint16_t( context.width )
			, uint16_t( context.height )
			, false
			, 1
			, depthFormat
			, textureFlags
		);

		context.fbh = bgfx::createFrameBuffer( BX_COUNTOF( context.fbTextureHandles ), context.fbTextureHandles, true );
		bgfx::ViewId rttView = 1;
		bgfx::setViewName( context.viewId, name );
		bgfx::setViewRect( context.viewId, 0, 0, bgfx::BackbufferRatio::Equal );
		bgfx::setViewFrameBuffer( context.viewId, context.fbh );
		bgfx::setViewClear( context.viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495FFFF, 1.0f, 0 );
	}
}

void bgfxRender( bgfxContext_t *context ){

	
	//setup frame buffer for rendertarget
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
            : bgfx::TextureFormat::D32;

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
        bgfx::setViewRect( rttView, 0, 0, bgfx::BackbufferRatio::Equal );
        bgfx::setViewFrameBuffer( rttView, context->fbh );

		float fov = 27.f;
		float camYAngle = 165.f / 180.f * 3.14159f;
		float camXAngle = 32.f / 180.f * 3.14159f;
		bx::Vec3 eye = { cosf( camYAngle ) * cosf( camXAngle ) * 10, sinf( camXAngle ) * 10, sinf( camYAngle ) * cosf( camXAngle ) * 10 };
		bx::Vec3 at = { 0.f, 0.f, 0.f };
		bx::Vec3 up = { 0.f, 1.f, 0.f };
		bx::mtxLookAt( context->cameraView.ToFloatPtr( ) ,eye,at,up, bx::Handness::Right);
    }

	bgfx::setViewTransform( 0, context->cameraView.ToFloatPtr( ), context->cameraProjection.ToFloatPtr( ) );
	//	BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_GEQUAL |
	//	BGFX_STATE_BLEND_FUNC(
	//		BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA );
	//bgfx::setState( state );
    float modelTransform[16];
    float modelRotation[16];
	float modelRotationY[16];
    float modelTranslation[16];
    float modelScale[16];
    float tmp[16];
	float tmp2[16];
    bx::mtxIdentity( modelTranslation );
	//bx::mtxRotateX( tmp2, abs( ( 0.001f * com_frameTime ) ));
	//bx::mtxRotateY( modelRotationY, abs( ( 0.001f * com_frameTime ) ) );
	//bx::mtxRotateZ( tmp2, abs( ( 0.001f * com_frameTime ) ) );
	bx::mtxRotateXYZ( modelRotation, abs( ( 0.001f * com_frameTime ) ), abs( ( 0.001f * com_frameTime ) ), 0.0f );
	//bx::mtxRotateXYZ( modelRotation, 0, abs( ( 0.001f * com_frameTime ) ), 0.0f );
    bx::mtxScale( modelScale, 0.8f + idMath::ClampFloat( 0.2f, 10.0f, ( abs( sin( 0.001f * com_frameTime ) ) ) ) );// sin( com_frameTime ) );

	//bx::mtxMul( modelRotation, tmp2, modelRotationY );

	idVec3 dir = idVec3(-1 , -1, 0 );
	dir.Normalize();
    idRotation modelRot( idVec3( 0, 0, 0 ), dir, RAD2DEG( abs( 0.001f * com_frameTime ) ) );
	idRotation modelRot2 (idVec3(0, 0, 0) ,idVec3(0, -1, 0) , RAD2DEG(abs( 0.001f * com_frameTime )) );
    idMat4 rotmat = modelRot.ToMat4();// * modelRot2.ToMat4( );

    float *xtmp;
    xtmp = rotmat.ToFloatPtr( );
	idMat4 cpy = rotmat;
	float *x2tmp = cpy.ToFloatPtr();
	
	//rotmat = ConvertFromIdSpace( cpy );
	//myGlMultMatrix( x2tmp, s_flipMatrix, xtmp );

    bx::mtxMul( tmp, modelScale, modelRotation );
    bx::mtxMul( modelTransform, tmp, modelTranslation );

	bx::mtxIdentity(modelTransform);
	bgfx::setTransform( modelTransform );
	bgfx::setVertexBuffer( 0, context->vbh );
	bgfx::setIndexBuffer( context->ibh );
	bgfx::submit( 0, context->program );
	//bx::mtxIdentity( modelTranslation );
	//bx::mtxMul( tmp, modelScale, xtmp );
	//bx::mtxMul( modelTransform, tmp, modelTranslation );
 //   bgfx::setTransform( modelTransform );
 //   bgfx::setVertexBuffer( 0, context->vbh );
 //   bgfx::setIndexBuffer( context->ibh );
 //   bgfx::submit( 1, context->program );
    
    for ( auto &item : bgfxCallbackList )
        item( context );

    if ( bgfx::isValid( context->rb ) )
        bgfx::blit( 2, context->rb, 0, 0, context->fbTextureHandle[0] );


}

void bgfxRegisterCallback( bgfxCallback callback )
{ 
    bgfxCallbackList.Append( callback );
}

int bgfxRenderThread( void *prunning ) {
	bool *running = ( bool * ) prunning;

	while ( ( *running ) ) {
        bgfx::renderFrame();
	}
	return 0;
}

void bgfxStartRenderThread( ) {
    if ( !RenderThread.threadHandle ) {
        RenderThreadRunning = true;
        Sys_CreateThread( bgfxRenderThread, &RenderThreadRunning, RenderThread, "BgfxRenderThread" );
    } else {
        common->Printf( "background thread already running\n" );
    }
}


