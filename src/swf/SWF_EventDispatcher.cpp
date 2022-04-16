
/*
===========================================================================

Copyright (C) 2022 HvanGinneken

===========================================================================
*/
#pragma hdrstop
#include "swf.h"
#include "../idFramework/idlib/containers/StrList.h"
#include "SWF_EventDispatcher.h"

/*
========================
idSWFScriptObject_EventDispatcherPrototype
========================
*/
#define SWF_EVENTDISPATCHER_FUNCTION_DEFINE( x ) idSWFScriptVar idSWFScriptObject_EventDispatcherPrototype::idSWFScriptFunction_##x::Call( idSWFScriptObject * thisObject, const idSWFParmList & parms )
#define SWF_EVENTDISPATCHER_NATIVE_VAR_DEFINE_GET( x ) idSWFScriptVar idSWFScriptObject_EventDispatcherPrototype::idSWFScriptNativeVar_##x::Get( class idSWFScriptObject * object )
#define SWF_EVENTDISPATCHER_NATIVE_VAR_DEFINE_SET( x ) void  idSWFScriptObject_EventDispatcherPrototype::idSWFScriptNativeVar_##x::Set( class idSWFScriptObject * object, const idSWFScriptVar & value )

#define SWF_EVENTDISPATCHER_PTHIS_FUNC( x ) idSWFScriptObject * pThis = thisObject ? thisObject : NULL; if ( !verify( pThis != NULL ) ) { idLib::Warning( "SWF: tried to call " x " on NULL object" ); return idSWFScriptVar(); }
#define SWF_EVENTDISPATCHER_PTHIS_GET( x ) idSWFScriptObject * pThis = object ? object : NULL; if ( pThis == NULL ) { return idSWFScriptVar(); }
#define SWF_EVENTDISPATCHER_PTHIS_SET( x ) idSWFScriptObject * pThis = object ? object : NULL; if ( pThis == NULL ) { return; }

#define SWF_EVENTDISPATCHER_FUNCTION_SET( x ) scriptFunction_##x.AddRef(); Set( #x, &scriptFunction_##x );
#define SWF_EVENTDISPATCHER_NATIVE_VAR_SET( x ) SetNative( #x, &swfScriptVar_##x );


idSWFScriptObject_EventDispatcherPrototype::idSWFScriptObject_EventDispatcherPrototype()
{
	SWF_EVENTDISPATCHER_NATIVE_VAR_SET ( MouseEvent );
	SWF_EVENTDISPATCHER_NATIVE_VAR_SET ( Event );
	SWF_EVENTDISPATCHER_FUNCTION_SET( addEventListener );
}
SWF_EVENTDISPATCHER_NATIVE_VAR_DEFINE_SET( MouseEvent ){}
SWF_EVENTDISPATCHER_NATIVE_VAR_DEFINE_SET( Event ){}

SWF_EVENTDISPATCHER_NATIVE_VAR_DEFINE_GET( MouseEvent ) { 
	static idSWFScriptObject * mouseEventObj = nullptr;
	if (mouseEventObj == nullptr)
	{
		mouseEventObj = idSWFScriptObject::Alloc( );
		mouseEventObj->Set( "CLICK", "MouseEvent.click" );
		mouseEventObj->Set( "CONTEXT_MENU", "MouseEvent.contextMenu" );
		mouseEventObj->Set( "DOUBLE_CLICK", "MouseEvent.doubleClick" );
		mouseEventObj->Set( "MIDDLE_CLICK", "MouseEvent.middleClick" );
		mouseEventObj->Set( "MIDDLE_MOUSE_DOWN", "MouseEvent.middleMouseDown" );
		mouseEventObj->Set( "MIDDLE_MOUSE_UP", "MouseEvent.middleMouseUp" );
		mouseEventObj->Set( "MOUSE_DOWN", "MouseEvent.mouseDown" );
		mouseEventObj->Set( "MOUSE_MOVE", "MouseEvent.mouseMove" );
		mouseEventObj->Set( "MOUSE_OUT", "MouseEvent.mouseOut" );
		mouseEventObj->Set( "MOUSE_OVER", "MouseEvent.mouseOver" );
		mouseEventObj->Set( "MOUSE_UP", "MouseEvent.mouseUp" );
		mouseEventObj->Set( "MOUSE_WHEEL ", "MouseEvent.mouseWheel" );
		mouseEventObj->Set( "RELEASE_OUTSIDE", "MouseEvent.releaseOutside" );
		mouseEventObj->Set( "RIGHT_CLICK", "MouseEvent.rightClick" );
		mouseEventObj->Set( "RIGHT_MOUSE_DOWN", "MouseEvent.rightMouseDown" );
		mouseEventObj->Set( "RIGHT_MOUSE_UP", "MouseEvent.rightMouseUp" );
		mouseEventObj->Set( "ROLL_OUT", "MouseEvent.rollOut" );
		mouseEventObj->Set( "ROLL_OVER", "MouseEvent.rollOver" );
	}
	return mouseEventObj;
}

SWF_EVENTDISPATCHER_NATIVE_VAR_DEFINE_GET( Event ) {
	static idSWFScriptObject *eventObj = nullptr;
	if ( eventObj == nullptr ) 	{
		eventObj = idSWFScriptObject::Alloc( );
		eventObj->Set( "ACTIVATE", "Event.activate" );
		eventObj->Set( "ADDED", "Event.added" );
		eventObj->Set( "ADDED_TO_STAGE", "Event.addedToStage" );
		eventObj->Set( "BROWSER_ZOOM_CHANGE", "Event.browserZoomChange" );
		eventObj->Set( "CANCEL", "Event.cancel" );
		eventObj->Set( "CHANGE", "Event.change" );
		eventObj->Set( "CHANNEL_MESSAGE", "Event.channelMessage" );
		eventObj->Set( "CHANNEL_STATE", "Event.channelState" );
		eventObj->Set( "CLEAR", "Event.clear" );
		eventObj->Set( "CLOSE", "Event.close" );
		eventObj->Set( "CLOSING", "Event.closing" );
		eventObj->Set( "COMPLETE", "Event.complete" );
		eventObj->Set( "CONNECT", "Event.connect" );
		eventObj->Set( "CONTEXT3D_CREATE", "Event.context3DCreate" );
		eventObj->Set( "COPY", "Event.copy" );
		eventObj->Set( "CUT", "Event.cut" );
		eventObj->Set( "DEACTIVATE", "Event.deactivate" );
		eventObj->Set( "DISPLAYING", "Event.displaying" );
		eventObj->Set( "ENTER_FRAME", "Event.enterFrame" );
		eventObj->Set( "EXIT_FRAME", "Event.exitFrame" );
		eventObj->Set( "EXITING", "Event.exiting" );
		eventObj->Set( "FRAME_CONSTRUCTED", "Event.frameConstructed" );
		eventObj->Set( "FRAME_LABEL", "Event.frameLabel" );
		eventObj->Set( "FULLSCREEN", "Event.fullScreen" );
		eventObj->Set( "HTML_BOUNDS_CHANGE", "Event.htmlBoundsChange" );
		eventObj->Set( "HTML_DOM_INITIALIZE", "Event.htmlDOMInitialize" );
		eventObj->Set( "HTML_RENDER", "Event.htmlRender" );
		eventObj->Set( "ID3", "Event.id3" );
		eventObj->Set( "INIT", "Event.init" );
		eventObj->Set( "LOCATION_CHANGE", "Event.locationChange" );
		eventObj->Set( "MOUSE_LEAVE", "Event.mouseLeave" );
		eventObj->Set( "NETWORK_CHANGE", "Event.networkChange" );
		eventObj->Set( "OPEN", "Event.open" );
		eventObj->Set( "PASTE", "Event.paste" );
		eventObj->Set( "PREPARING", "Event.preparing" );
		eventObj->Set( "REMOVED", "Event.removed" );
		eventObj->Set( "REMOVED_FROM_STAGE", "Event.removedFromStage" );
		eventObj->Set( "RENDER", "Event.render" );
		eventObj->Set( "RESIZE", "Event.resize" );
		eventObj->Set( "SCROLL", "Event.scroll" );
		eventObj->Set( "SELECT", "Event.select" );
		eventObj->Set( "SELECT_ALL", "Event.selectAll" );
		eventObj->Set( "SOUND_COMPLETE", "Event.soundComplete" );
		eventObj->Set( "STANDARD_ERROR_CLOSE", "Event.standardErrorClose" );
		eventObj->Set( "STANDARD_INPUT_CLOSE", "Event.standardInputClose" );
		eventObj->Set( "STANDARD_OUTPUT_CLOSE", "Event.standardOutputClose" );
		eventObj->Set( "SUSPEND", "Event.suspend" );
		eventObj->Set( "TAB_CHILDREN_CHANGE", "Event.tabChildrenChange" );
		eventObj->Set( "TAB_ENABLED_CHANGE", "Event.tabEnabledChange" );
		eventObj->Set( "TAB_INDEX_CHANGE", "Event.tabIndexChange" );
		eventObj->Set( "TEXT_INTERACTION_MODE_CHANGE", "Event.textInteractionModeChange" );
		eventObj->Set( "TEXTURE_READY", "Event.textureReady" );
		eventObj->Set( "UNLOAD", "Event.unload" );
		eventObj->Set( "USER_IDLE", "Event.userIdle" );
		eventObj->Set( "USER_PRESENT", "Event.userPresent" );
		eventObj->Set( "VIDEO_FRAME", "Event.videoFrame" );
		eventObj->Set( "WORKER_STATE", "Event.workerState" );
	}
	return eventObj;
}

SWF_EVENTDISPATCHER_FUNCTION_DEFINE( addEventListener ) 
{
	SWF_EVENTDISPATCHER_PTHIS_FUNC( "addEventListener" );
	thisObject->Set("__"+parms[1].ToString()+"__",parms[0]);
	common->DPrintf("[%s] AddEventListener(%s,%s)\n", thisObject->GetSprite()->name.c_str(),parms[1].ToString().c_str(),parms[0].ToString().c_str());
	//add listener
	return idSWFScriptVar( );
}


