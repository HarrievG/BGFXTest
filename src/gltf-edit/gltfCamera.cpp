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
	auto& entry = GetOverride(cameraID);
	
	if ( entry == EmptOverrideEntry && data->CameraList().Num() >= cameraID)
		return Override(data->CameraList()[cameraID]);

	return entry;

}

gltfCameraManager::OverrideEntry &gltfCameraManager::Override( gltfCamera *camera ) {
	gltfNode * res = data->GetCameraNodes( camera );
	OverrideEntry &newCam = overrides.Alloc( );
	newCam.originalCameraID = data->CameraList().FindIndex(camera);
	newCam.newNodeID = data->NodeList( ).Num( );
	newCam.originalNodeID = data->NodeList().FindIndex(res);
	gltfNode *newCameraNode = data->Node( );
	
	//so this is only correct when the camera os not a child of something else.
	newCameraNode->rotation = res->rotation;
	newCameraNode->translation = res->translation;
	newCameraNode->scale = res->scale;

	newCam.newCameraID = data->CameraList( ).Num( );
	gltfCamera *newCamera = data->Camera( );
	*newCamera = *camera;
	newCamera->name = "_override_" + camera->name;
	newCam.originalCameraID =  res->camera;
	newCameraNode->camera = newCam.newCameraID;
	newCameraNode->name = "_override_" + res->name;
	newCameraNode->dirty =true;

	return newCam;
}

gltfCameraManager::OverrideEntry &gltfCameraManager::GetOverride( int cameraID, bool searchOwner ) {
	for ( auto &entry : overrides ) {
		if ( entry.originalCameraID == cameraID || (searchOwner && entry.newCameraID == cameraID)) {
			return entry;
		}
	}

	return EmptOverrideEntry;
}

bool gltfCameraManager::HasOverideID( int cameraID ) {
	auto &entry = GetOverride( cameraID );
	return entry != EmptOverrideEntry;
}

bool gltfCameraManager::IsOverride( int cameraID ) {
	int count = 0;
	for ( auto &entry : overrides ) {
		if ( entry.newCameraID == cameraID ) {
			return true;
		}
	}
	return false;
}

bool gltfCameraManager::IsEmtpy( const OverrideEntry &entry ) {
	return entry == gltfCameraManager::EmptOverrideEntry;
}

