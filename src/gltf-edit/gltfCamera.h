#pragma once
#include "../idFramework/idlib/containers/HashIndex.h"
#include "../idFramework/idlib/containers/List.h"

class gltfCamera;
class gltfData;
class gltfCameraManager {
public:

	struct OverrideEntry {
		bool operator==( const gltfCameraManager::OverrideEntry & rhs ) const ;
		OverrideEntry( ) : originalCameraID( -1 ), newCameraID( -1 ), originalNodeID( -1 ), newNodeID( -1 ) { }
		int originalCameraID;
		int newCameraID;
		int	originalNodeID;
		int newNodeID;
	};

	gltfCameraManager( gltfData *_data ) : data( _data ) { }
	~gltfCameraManager( ) { };

	//returns the index of an new cameraNode 
	// this copies all of the data from the given camera
	// if an _orientation node is found, it will me be merged into the new Node
	OverrideEntry & Override( gltfCamera * camera );
	OverrideEntry & Override( int cameraID );
	OverrideEntry & GetOverride( int cameraID );
	bool HasOverideID( int cameraID ) ;
	bool IsEmtpy( const OverrideEntry & entry );
	OverrideEntry EmptOverrideEntry;
private:
	gltfData *data;
	idList<OverrideEntry>	overrides;
	idList<gltfCamera *>	cameraOverride;
	idHashIndex				cameraOverrideIndices;
};