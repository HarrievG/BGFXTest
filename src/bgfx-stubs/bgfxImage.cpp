#include "bgfxImage.h"
#include "stb_image.h"
#include "gltf-edit/gltfParser.h"

xthreadInfo imageLoadThread;
bool		imageLoadThreadRunning = false;
bool		dummyTextureCreated = false;
bgfx::TextureHandle  sDummyTextureHandle;
idList<imageLoad_t*> loadQueue;
SDL_semaphore * loadSem = nullptr;

void SetTextureLoadDummy ( )
{
	if ( dummyTextureCreated )
		return;

	stbi_uc dummy[4*4];

	for (int i = 0; i < 16; i+=4 )
	{
		idRandom rnd( i );
		int r = rnd.RandomInt( 255 ), g = rnd.RandomInt( 255 ), b = rnd.RandomInt( 255 );
		dummy[i+0] = 255;
		dummy[i+1] = 255;
		dummy[i+2] = 255;
		dummy[i+3] = 255;
	}

	uint32_t tex_flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_U_BORDER | BGFX_SAMPLER_V_BORDER;//add point and repeat
	sDummyTextureHandle = bgfx::createTexture2D( 2,2, false, 1, bgfx::TextureFormat::RGBA8, tex_flags, bgfx::copy( dummy, 4*4 ) );
	dummyTextureCreated = true;
}


bgfxTextureHandle bgfxImageLoad( byte *data, size_t length ) {

	int width, height, channels;
	bgfxTextureHandle ret;
	ret.handle.idx = -1;

	stbi_uc *imageData = stbi_load_from_memory( ( stbi_uc const * ) data, length, &width, &height, &channels, STBI_rgb_alpha );
	if ( imageData != NULL ) {
		uint32_t tex_flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;//add point and repeat
		ret.handle = bgfx::createTexture2D( width, height, false, 1, bgfx::TextureFormat::RGBA8, tex_flags, bgfx::copy( imageData, width * height * 4 ) );
		ret.dim.x = width;
		ret.dim.y = height;
		ret.loaded = true;
		stbi_image_free( imageData );
	} else
		common->Warning( " FAILED TO LOAD TEXTURE " );

	return ret;
}

void bgfxImageLoad( byte *data, size_t length, bgfxTextureHandle *handle, uint32_t flags) {

	int width, height, channels;

	stbi_uc *imageData = stbi_load_from_memory( ( stbi_uc const * ) data, length, &width, &height, &channels, STBI_rgb_alpha );
	if ( imageData != NULL ) 	
	{
		handle->handle = bgfx::createTexture2D( width, height, false, 1, bgfx::TextureFormat::RGBA8, flags, bgfx::copy( imageData, width * height * 4 ) );
		handle->dim.x = width;
		handle->dim.y = height;
		handle->loaded = true;
		stbi_image_free( imageData );
	}
	else
		common->Warning( " FAILED TO LOAD TEXTURE " );
}

imageLoad_t * GetNextImage()
{
	Sys_EnterCriticalSection( CRITICAL_SECTION_IMAGE_LOAD );

	if (loadQueue.Num() < 1 )
		return nullptr;

	int currentIndex = loadQueue.Num( ) - 1;
	imageLoad_t *next = loadQueue[ currentIndex ];
	loadQueue.RemoveIndex( currentIndex );

	Sys_LeaveCriticalSection ( CRITICAL_SECTION_IMAGE_LOAD );

	return next;
}

//targetHandle will be written from another thread!
void bgfxImageLoadAsync( byte *data, size_t length, bgfxTextureHandle * targetHandle, uint32_t flags ) {
	Sys_EnterCriticalSection( CRITICAL_SECTION_IMAGE_LOAD );
	
	loadQueue.AssureSizeAlloc( loadQueue.Num() + 1, idListNewElement<imageLoad_t> );
	imageLoad_t *next = loadQueue[loadQueue.Num( ) -1] ;
	next->data = data;
	next->length = length;
	next->targetHandle = targetHandle;
	next->flags = flags;
	targetHandle->handle = sDummyTextureHandle;
	Sys_LeaveCriticalSection( CRITICAL_SECTION_IMAGE_LOAD );

	SDL_SemPost(loadSem);
	Sys_TriggerEvent( TRIGGER_EVENT_IMAGE_LOAD );
}

int bgfxImageLoadThread( void *prunning ) {
	bool *running = ( bool * ) prunning;
	//this start really early, and the loading thread should actually be starting after intialization and the first drawn frame.
	//but for now, lets just wait a bit
	while ( ( *running ) ) {
		SDL_SemWait(loadSem);
		imageLoad_t *next = GetNextImage( );
		if ( next )
			bgfxImageLoad( next->data, next->length,next->targetHandle,next->flags );
	}
	return 0;
}

void bgfxStartImageLoadThread() {
	if ( !imageLoadThreadRunning) {
		loadSem = SDL_CreateSemaphore(0);
		SetTextureLoadDummy();
		imageLoadThreadRunning = true;
		Sys_CreateThread( bgfxImageLoadThread, &imageLoadThreadRunning, imageLoadThread, "BgfxImageLoadThread" );
	} else {
		common->Printf( "background thread already running\n" );
	}
}
