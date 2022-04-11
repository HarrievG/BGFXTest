#include <bgfx/bgfx.h>
#include "idFramework/Common.h"

idCVar bgfx_verbose( "bgfx_verbose", "0",  CVAR_RENDERER | CVAR_INTEGER, "0 = no output, 1 = message and warnings only, 2 = warnings only, 3 = messages, warnings and location, ",0,3, idCmdSystem::ArgCompletion_Integer<0, 3> );
idCVar bgfx_nonFatal( "bgfx_nonFatal", "0",  CVAR_RENDERER | CVAR_INTEGER, "1 = turn into debug warning and keep running, 0 = throw fatal error, ",0,2, idCmdSystem::ArgCompletion_Integer<0, 3> );
//https://github.com/bkaradzic/bgfx/blob/master/examples/07-callback/callback.cpp
namespace bgfx {
struct CallbackStub : public CallbackI 	{
	virtual ~CallbackStub( ) 		{ 		}

	virtual void fatal( const char *_filePath, uint16_t _line, Fatal::Enum _code, const char *_str ) override {

		__debugbreak();
		switch( bgfx_verbose.GetInteger() ) {
			case 0: // always show fatal errors.
			default:
			case 1:
			case 2:
				if( bgfx_nonFatal.GetInteger() )
					common->FatalError( "[BGFX] FATAL 0x%08x: %s",_code,_str);
				else
					common->DWarning( "[BGFX] FATAL 0x%08x: %s",_code,_str);
				break;
			case 3:
				if( bgfx_nonFatal.GetInteger() )
					common->FatalError( "[BGFX] %s,%i, FATAL 0x%08x: %s\n", _filePath, _line, _code, _str );
				else
					common->DWarning( "[BGFX] %s,%i, FATAL 0x%08x: %s\n", _filePath, _line, _code, _str );
				break;
		}
	}

	virtual void traceVargs( const char *_filePath, uint16_t _line, const char *_format, va_list _argList ) override {
		bool warning = false;

		if (!bgfx_verbose.GetInteger( ))
			return;

		char msg[4096];
		idStr::vsnPrintf( msg, 4096, _format, _argList );

		if (!common->IsInitialized())
		{

			Sys_Printf( msg );
			return;
		}
		const static idStr wrnStr( "WARN" );
		if ( strlen( msg ) >= wrnStr.Length())
			warning = idStr::FindText(msg,wrnStr) != -1;


		switch ( bgfx_verbose.GetInteger( ) ) {
		case 0:
			break;
		default:
		case 1:
			if ( warning )
				common->VWarning( _format, _argList );
			else
				common->VPrintf(_format,_argList);
			break;
		case 2:
			if ( warning ) 
				common->VWarning( _format, _argList );
			break;
		case 3:
			if ( warning )
			{
				common->Warning( "%s,%i ", _filePath, _line );
				common->VWarning( _format, _argList );
			} else 
			{
				common->Printf( "%s,%i ", _filePath, _line );
				common->VPrintf( _format, _argList );
			}

			break;
		}
	}

	virtual void profilerBegin( const char * /*_name*/, uint32_t /*_abgr*/, const char * /*_filePath*/, uint16_t /*_line*/ ) override {
		common->DWarning( "[BGFX] profilerBegin not implemented" );
	}

	virtual void profilerBeginLiteral( const char * /*_name*/, uint32_t /*_abgr*/, const char * /*_filePath*/, uint16_t /*_line*/ ) override {
		common->DWarning( "[BGFX] profilerBeginLiteral not implemented" );
	}

	virtual void profilerEnd( ) override { 
		common->DWarning( "[BGFX] profilerEnd not implemented" ); 
	}

	virtual uint32_t cacheReadSize( uint64_t /*_id*/ ) override 		{
		common->DWarning( "[BGFX] cacheReadSize not implemented" );
		return 0;
	}

	virtual bool cacheRead( uint64_t /*_id*/, void * /*_data*/, uint32_t /*_size*/ ) override {
		common->DWarning( "[BGFX] cacheRead not implemented" );
		return false;
	}

	virtual void cacheWrite( uint64_t /*_id*/, const void * /*_data*/, uint32_t /*_size*/ ) override {
		common->DWarning( "[BGFX] cacheWrite not implemented" );
	}

	virtual void screenShot( const char *_filePath, uint32_t _width, uint32_t _height, uint32_t _pitch, const void *_data, uint32_t _size, bool _yflip ) override {
		common->DWarning( "[BGFX] screenShot not implemented" );
	}

	virtual void captureBegin( uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_pitch*/, TextureFormat::Enum /*_format*/, bool /*_yflip*/ ) override {
		common->DWarning("[BGFX] using capture without callback (a.k.a. pointless)." );
	}

	virtual void captureEnd( ) override { 
		common->DWarning( "[BGFX] captureEnd not implemented\n" ); 
	}

	virtual void captureFrame( const void * /*_data*/, uint32_t /*_size*/ ) override { 
		common->DWarning( "[BGFX] captureFrame not implemented\n" ); 
	}
};
CallbackStub bgfxCallbacksLocal;
} //nampespace bgfx