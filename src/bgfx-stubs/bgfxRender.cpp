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

idCVar r_customWidth( "r_customWidth", "1920", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, "custom screen width. set r_mode to -1 to activate" );
idCVar r_customHeight( "r_customHeight", "1080", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, "custom screen height. set r_mode to -1 to activate" );
idCVar r_aspectRatio( "r_aspectRatio", "-1", CVAR_RENDERER | CVAR_INTEGER | CVAR_ARCHIVE, "aspect ratio of view:\n0 = 4:3\n1 = 16:9\n2 = 16:10\n-1 = auto (guess from resolution)", -1, 2 );
idCVar r_mode( "r_mode", "-1", CVAR_ARCHIVE | CVAR_RENDERER | CVAR_INTEGER, "video mode number" );

/*
======================
idScreenRect::Clear
======================
*/
void idScreenRect::Clear() {
	x1 = y1 = 32000;
	x2 = y2 = -32000;
	zmin = 0.0f; zmax = 1.0f;
}

/*
======================
idScreenRect::AddPoint
======================
*/
void idScreenRect::AddPoint( float x, float y ) {
	int	ix = idMath::FtoiFast( x );
	int iy = idMath::FtoiFast( y );

	if ( ix < x1 ) {
		x1 = ix;
	}
	if ( ix > x2 ) {
		x2 = ix;
	}
	if ( iy < y1 ) {
		y1 = iy;
	}
	if ( iy > y2 ) {
		y2 = iy;
	}
}

/*
======================
idScreenRect::Expand
======================
*/
void idScreenRect::Expand() {
	x1--;
	y1--;
	x2++;
	y2++;
}

/*
======================
idScreenRect::Intersect
======================
*/
void idScreenRect::Intersect( const idScreenRect &rect ) {
	if ( rect.x1 > x1 ) {
		x1 = rect.x1;
	}
	if ( rect.x2 < x2 ) {
		x2 = rect.x2;
	}
	if ( rect.y1 > y1 ) {
		y1 = rect.y1;
	}
	if ( rect.y2 < y2 ) {
		y2 = rect.y2;
	}
}

/*
======================
idScreenRect::Union
======================
*/
void idScreenRect::Union( const idScreenRect &rect ) {
	if ( rect.x1 < x1 ) {
		x1 = rect.x1;
	}
	if ( rect.x2 > x2 ) {
		x2 = rect.x2;
	}
	if ( rect.y1 < y1 ) {
		y1 = rect.y1;
	}
	if ( rect.y2 > y2 ) {
		y2 = rect.y2;
	}
}

/*
======================
idScreenRect::Equals
======================
*/
bool idScreenRect::Equals( const idScreenRect &rect ) const {
	return ( x1 == rect.x1 && x2 == rect.x2 && y1 == rect.y1 && y2 == rect.y2 );
}

/*
======================
idScreenRect::IsEmpty
======================
*/
bool idScreenRect::IsEmpty() const {
	return ( x1 > x2 || y1 > y2 );
}

void bgfxRenderViewToViewport(bgfxContext_t context, idScreenRect *viewport ) {

    float wRatio = ( float ) context.width / SCREEN_WIDTH;
    float hRatio = ( float ) context.height / SCREEN_HEIGHT;

    viewport->x1 = idMath::Ftoi( context.x * wRatio );
    viewport->x2 = idMath::Ftoi( floor( ( context.x + context.width ) * wRatio + 0.5f ) - 1 );
    viewport->y1 = idMath::Ftoi( context.height - floor( ( context.y + context.height ) * hRatio + 0.5f ) );
    viewport->y2 = idMath::Ftoi( context.height - floor( context.y * hRatio + 0.5f ) - 1 );
}

void bgfxCalcFov( float base_fov, float &fov_x, float &fov_y ) {
	float	x;
	float	y;
	float	ratio_x;
	float	ratio_y;

	// first, calculate the vertical fov based on a 640x480 view
	x = 640.0f / tan( base_fov / 360.0f * idMath::PI );
	y = atan2( 480.0f, x );
	fov_y = y * 360.0f / idMath::PI;

	// FIXME: somehow, this is happening occasionally
	assert( fov_y > 0 );
	if ( fov_y <= 0 ) {
		common->Error( "idGameLocal::CalcFov: bad result, fov_y == %f, base_fov == %f", fov_y, base_fov );
	}

	switch( r_aspectRatio.GetInteger() ) {
	default :
	case -1 :
		// auto mode => use aspect ratio from resolution, assuming screen's pixels are squares
		ratio_x = r_customWidth.GetInteger();
		ratio_y = r_customHeight.GetInteger( );
		if(ratio_x <= 0.0f || ratio_y <= 0.0f)
		{
			// for some reason (maybe this is a dedicated server?) GetScreenWidth()/Height()
			// returned 0. Assume default 4:3 to avoid assert()/Error() below.
			fov_x = base_fov;
			return;
		}
		break;
	case 0 :
		// 4:3
		fov_x = base_fov;
		return;
		break;

	case 1 :
		// 16:9
		ratio_x = 16.0f;
		ratio_y = 9.0f;
		break;

	case 2 :
		// 16:10
		ratio_x = 16.0f;
		ratio_y = 10.0f;
		break;
	}

	y = ratio_y / tan( fov_y / 360.0f * idMath::PI );
	fov_x = atan2( ratio_x, y ) * 360.0f / idMath::PI;

	if ( fov_x < base_fov ) {
		fov_x = base_fov;
		x = ratio_x / tan( fov_x / 360.0f * idMath::PI );
		fov_y = atan2( ratio_y, x ) * 360.0f / idMath::PI;
	}

	// FIXME: somehow, this is happening occasionally
	assert( ( fov_x > 0 ) && ( fov_y > 0 ) );
	if ( ( fov_y <= 0 ) || ( fov_x <= 0 ) ) {
		common->Error( "idGameLocal::CalcFov: bad result" );
	}
}

idList<bgfxCallback> bgfxCallbackList;
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
    
    bgfxCallbackList.Clear( );

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

    idMat4 camMat;

    idRotation camRot;
    camRot.SetVec( context->cam_pitch, context->cam_yaw, 0.0f );
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

	static float	s_flipMatrix[16] = {
		// convert from our coordinate system (looking down X)
		// to OpenGL's coordinate system (looking down -Z)
		0, 0, -1, 0,
		-1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 1
	};

    float *xtmp;
    xtmp = rotmat.ToFloatPtr( );
	idMat4 cpy = rotmat;
	float *x2tmp = cpy.ToFloatPtr();
	
	//rotmat = ConvertFromIdSpace( cpy );
	//myGlMultMatrix( x2tmp, s_flipMatrix, xtmp );

    bx::mtxMul( tmp, modelScale, modelRotation );
    bx::mtxMul( modelTransform, tmp, modelTranslation );
	bgfx::setTransform( modelTransform );
	bgfx::setVertexBuffer( 0, context->vbh );
	bgfx::setIndexBuffer( context->ibh );
	bgfx::submit( 0, context->program );

	bx::mtxIdentity( modelTranslation );
	bx::mtxMul( tmp, modelScale, xtmp );
	bx::mtxMul( modelTransform, tmp, modelTranslation );
    bgfx::setTransform( modelTransform );
    bgfx::setVertexBuffer( 0, context->vbh );
    bgfx::setIndexBuffer( context->ibh );
    bgfx::submit( 1, context->program );
    
    for ( auto &item : bgfxCallbackList )
        item( context );

    if ( bgfx::isValid( context->rb ) )
        bgfx::blit( 2, context->rb, 0, 0, context->fbTextureHandle[0] );


}

void bgfxRegisterCallback( bgfxCallback callback )
{ 
    bgfxCallbackList.Append( callback );
}

/*
===============
ConvertToIdSpace
===============
*/
idMat3 ConvertToIdSpace( const idMat3 &mat ) {
	idMat3 idmat;

	idmat[ 0 ][ 0 ] = mat[ 0 ][ 0 ];
	idmat[ 0 ][ 1 ] = -mat[ 0 ][ 2 ];
	idmat[ 0 ][ 2 ] = mat[ 0 ][ 1 ];

	idmat[ 1 ][ 0 ] = mat[ 1 ][ 0 ];
	idmat[ 1 ][ 1 ] = -mat[ 1 ][ 2 ];
	idmat[ 1 ][ 2 ] = mat[ 1 ][ 1 ];

	idmat[ 2 ][ 0 ] = mat[ 2 ][ 0 ];
	idmat[ 2 ][ 1 ] = -mat[ 2 ][ 2 ];
	idmat[ 2 ][ 2 ] = mat[ 2 ][ 1 ];

	return idmat;
}

/*
===============
ConvertToIdSpace
===============
*/
idVec3 ConvertToIdSpace( const idVec3 &pos ) {
	idVec3 idpos;

	idpos.x = pos.x;
	idpos.y = -pos.z;
	idpos.z = pos.y;

	return idpos;
}

/*
===============
ConvertFromIdSpace
===============
*/
idMat3 ConvertFromIdSpace( const idMat3 &idmat ) {
	idMat3 mat;

	mat[ 0 ][ 0 ] = idmat[ 0 ][ 0 ];
	mat[ 0 ][ 2 ] = -idmat[ 0 ][ 1 ];
	mat[ 0 ][ 1 ] = idmat[ 0 ][ 2 ];

	mat[ 1 ][ 0 ] = idmat[ 1 ][ 0 ];
	mat[ 1 ][ 2 ] = -idmat[ 1 ][ 1 ];
	mat[ 1 ][ 1 ] = idmat[ 1 ][ 2 ];

	mat[ 2 ][ 0 ] = idmat[ 2 ][ 0 ];
	mat[ 2 ][ 2 ] = -idmat[ 2 ][ 1 ];
	mat[ 2 ][ 1 ] = idmat[ 2 ][ 2 ];

	return mat;
}

idMat4 ConvertFromIdSpace( const idMat4 &idmat ) {
	idMat4 mat;

	mat[ 0 ][ 0 ] = idmat[ 0 ][ 0 ];
	mat[ 0 ][ 2 ] = -idmat[ 0 ][ 1 ];
	mat[ 0 ][ 1 ] = idmat[ 0 ][ 2 ];

	mat[ 1 ][ 0 ] = idmat[ 1 ][ 0 ];
	mat[ 1 ][ 2 ] = -idmat[ 1 ][ 1 ];
	mat[ 1 ][ 1 ] = idmat[ 1 ][ 2 ];

	mat[ 2 ][ 0 ] = idmat[ 2 ][ 0 ];
	mat[ 2 ][ 2 ] = -idmat[ 2 ][ 1 ];
	mat[ 2 ][ 1 ] = idmat[ 2 ][ 2 ];

    mat[ 3 ][ 0 ] = idmat[ 3 ][ 0 ];
	mat[ 3 ][ 2 ] = -idmat[ 3 ][ 1 ];
	mat[ 3 ][ 1 ] = idmat[ 3 ][ 2 ];

	return mat;
}


/*
==========================
myGlMultMatrix
==========================
*/
void myGlMultMatrix( const float a[16], const float b[16], float out[16] ) {
#if 0
	int		i, j;

	for ( i = 0 ; i < 4 ; i++ ) {
		for ( j = 0 ; j < 4 ; j++ ) {
			out[ i * 4 + j ] =
				a [ i * 4 + 0 ] * b [ 0 * 4 + j ]
				+ a [ i * 4 + 1 ] * b [ 1 * 4 + j ]
				+ a [ i * 4 + 2 ] * b [ 2 * 4 + j ]
				+ a [ i * 4 + 3 ] * b [ 3 * 4 + j ];
		}
	}
#else
	out[0*4+0] = a[0*4+0]*b[0*4+0] + a[0*4+1]*b[1*4+0] + a[0*4+2]*b[2*4+0] + a[0*4+3]*b[3*4+0];
	out[0*4+1] = a[0*4+0]*b[0*4+1] + a[0*4+1]*b[1*4+1] + a[0*4+2]*b[2*4+1] + a[0*4+3]*b[3*4+1];
	out[0*4+2] = a[0*4+0]*b[0*4+2] + a[0*4+1]*b[1*4+2] + a[0*4+2]*b[2*4+2] + a[0*4+3]*b[3*4+2];
	out[0*4+3] = a[0*4+0]*b[0*4+3] + a[0*4+1]*b[1*4+3] + a[0*4+2]*b[2*4+3] + a[0*4+3]*b[3*4+3];
	out[1*4+0] = a[1*4+0]*b[0*4+0] + a[1*4+1]*b[1*4+0] + a[1*4+2]*b[2*4+0] + a[1*4+3]*b[3*4+0];
	out[1*4+1] = a[1*4+0]*b[0*4+1] + a[1*4+1]*b[1*4+1] + a[1*4+2]*b[2*4+1] + a[1*4+3]*b[3*4+1];
	out[1*4+2] = a[1*4+0]*b[0*4+2] + a[1*4+1]*b[1*4+2] + a[1*4+2]*b[2*4+2] + a[1*4+3]*b[3*4+2];
	out[1*4+3] = a[1*4+0]*b[0*4+3] + a[1*4+1]*b[1*4+3] + a[1*4+2]*b[2*4+3] + a[1*4+3]*b[3*4+3];
	out[2*4+0] = a[2*4+0]*b[0*4+0] + a[2*4+1]*b[1*4+0] + a[2*4+2]*b[2*4+0] + a[2*4+3]*b[3*4+0];
	out[2*4+1] = a[2*4+0]*b[0*4+1] + a[2*4+1]*b[1*4+1] + a[2*4+2]*b[2*4+1] + a[2*4+3]*b[3*4+1];
	out[2*4+2] = a[2*4+0]*b[0*4+2] + a[2*4+1]*b[1*4+2] + a[2*4+2]*b[2*4+2] + a[2*4+3]*b[3*4+2];
	out[2*4+3] = a[2*4+0]*b[0*4+3] + a[2*4+1]*b[1*4+3] + a[2*4+2]*b[2*4+3] + a[2*4+3]*b[3*4+3];
	out[3*4+0] = a[3*4+0]*b[0*4+0] + a[3*4+1]*b[1*4+0] + a[3*4+2]*b[2*4+0] + a[3*4+3]*b[3*4+0];
	out[3*4+1] = a[3*4+0]*b[0*4+1] + a[3*4+1]*b[1*4+1] + a[3*4+2]*b[2*4+1] + a[3*4+3]*b[3*4+1];
	out[3*4+2] = a[3*4+0]*b[0*4+2] + a[3*4+1]*b[1*4+2] + a[3*4+2]*b[2*4+2] + a[3*4+3]*b[3*4+2];
	out[3*4+3] = a[3*4+0]*b[0*4+3] + a[3*4+1]*b[1*4+3] + a[3*4+2]*b[2*4+3] + a[3*4+3]*b[3*4+3];
#endif
}
/*
===============
ConvertFromIdSpace
===============
*/
idVec3 ConvertFromIdSpace( const idVec3 &idpos ) {
	idVec3 pos;

	pos.x = idpos.x;
	pos.z = -idpos.y;
	pos.y = idpos.z;

	return pos;
}
