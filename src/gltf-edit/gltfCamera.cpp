#include "imgui.h"
#include "CVarSystem.h"
#include "Common.h"
#include "FileSystem.h"
#include "CVarSystem.h"
#include "bgfx/bgfx.h"
#include "bimg/bimg.h"
#include "gltfParser.h"
#include <ImGuizmo.h>
#include "gltfParser.h"
#include "gltfProperties.h"
#include "gltfCamera.h"


ID_INLINE bool gltfCameraManager::OverrideEntry::operator==( const gltfCameraManager::OverrideEntry &rhs ) const  {
	return ( newCameraID == rhs.newCameraID && originalCameraID == rhs.originalCameraID && originalNodeID == rhs.originalCameraID && newNodeID == rhs.newNodeID );
}


gltfCameraManager::OverrideEntry & gltfCameraManager::Override( int cameraID ) {
	auto entry = GetOverride(cameraID);
	
	if ( entry == EmptOverrideEntry && data->CameraList().Num() >= cameraID)
		return Override(data->CameraList()[cameraID]);

	return entry;

}

gltfCameraManager::OverrideEntry &gltfCameraManager::Override( gltfCamera *camera ) {
	gltfCameraNodePtrs res = data->GetCameraNodes( camera );
	OverrideEntry &newCam = overrides.Alloc( );

	newCam.newNodeID = data->NodeList( ).Num( );
	gltfNode *newCameraNode = data->Node( );
	
	newCameraNode->rotation = res.translationNode->rotation;
	newCameraNode->translation = res.translationNode->translation;

	newCam.newCameraID = data->CameraList( ).Num( );
	gltfCamera *newCamera = data->Camera( );

	newCam.originalCameraID =  res.translationNode->camera;
	newCameraNode->camera = newCam.newCameraID;
	newCameraNode->name = "_override_" + res.translationNode->name;
	if ( res.orientationNode ) {
		newCameraNode->rotation *= res.orientationNode->rotation;
		newCameraNode->translation += res.orientationNode->translation;
		newCameraNode->name = "_override_" + res.orientationNode->name;
		newCam.originalCameraID =  res.orientationNode->camera;
	}
	newCameraNode->dirty =true;

	return newCam;
}

gltfCameraManager::OverrideEntry &gltfCameraManager::GetOverride( int cameraID ) {
	int count = 0;
	for ( auto &entry : overrides ) {
		if ( entry.originalCameraID == cameraID ) {
			return entry;
		}
	}

	return EmptOverrideEntry;
}

bool gltfCameraManager::HasOverideID( int cameraID ) {
	auto &entry = GetOverride( cameraID );
	return entry != EmptOverrideEntry;
}

bool gltfCameraManager::IsEmtpy( const OverrideEntry &entry ) {
	return entry == gltfCameraManager::EmptOverrideEntry;
}

