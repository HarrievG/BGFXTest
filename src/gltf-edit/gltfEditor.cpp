#include "gltfEditor.h"
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// no need for it, it should go,  but doenst compile ootb..
//#define TINYGLTF_NO_FS
#include "tiny_gltf.h"
////////////////////////

#include "imgui.h"
#include "CVarSystem.h"
#include "Common.h"
#include "FileSystem.h"
#include "CVarSystem.h"
#include "bgfx/bgfx.h"
#include "bimg/bimg.h"
#include "gltfParser.h"

tinygltf::TinyGLTF gGLFTLoader;

static gltfSceneEditor localSceneEditor;
gltfSceneEditor *sceneEditor = &localSceneEditor;

static gltfAssetExplorer localAssetExplorer;
gltfAssetExplorer *assetExplorer = &localAssetExplorer;

static GLTF_Parser * gltfParser;


void gltfSceneEditor::Shutdown( )
{
	delete gltfParser;
}
gltfSceneEditor::gltfSceneEditor( )
	: windowOpen(false)
{
	tinygltf::FsCallbacks callbacks;
	callbacks.FileExists = [](const std::string &abs_filename, void *) 
		-> bool {
		if ( cvarSystem->GetCVarInteger( "fs_debug" ))
			common->DPrintf("tinyGLTF checks for %s\n",abs_filename.c_str() );
		return fileSystem->FindFile(abs_filename.c_str()) == findFile_t::FIND_YES;
	};
	callbacks.ExpandFilePath = [](const std::string &path, void *) 
		-> std::string {
		if ( cvarSystem->GetCVarInteger( "fs_debug" ))
			common->DPrintf( "tinyGLTF not expanding for %s\n", path.c_str( ) );
		return path;//std::string(fileSystem->RelativePathToOSPath( path.c_str())) ;
	};

	callbacks.ReadWholeFile = [] (std::vector<unsigned char> *data, std::string * err, const std::string & file,void * usr) 
		-> bool { 
		if ( cvarSystem->GetCVarInteger( "fs_debug" ))
			common->DPrintf( "tinyGLTF wants to read (%s)\n", file.c_str() );
		// dont stall to much.
		// all file loading should go to seperate thread and fire callback on completion.
		
		if (fileSystem->FindFile( file.c_str()) == findFile_t::FIND_YES )
		{
			int size = fileSystem->ReadFile( file.c_str(),NULL);
			if (!size )
			{
				*err = "File empty";
				return false;
			}
			if ( cvarSystem->GetCVarInteger( "fs_debug" ))
				common->DPrintf( "opening file %s %i bytes\n", file.c_str( ),size );
			const unsigned char *buffer = NULL;// reinterpret_cast< const unsigned char * >( buff );
			bool res = fileSystem->ReadFile( file.c_str( ), ( ( void ** ) &buffer ) ) > 0;
			*data = std::vector<unsigned char>( buffer,buffer + size);
			return res;
		}
		*err = "File not found";
		return false;
	};

	callbacks.WriteWholeFile = [] (std::string * a, const std::string & b, const std::vector<unsigned char> & c, void *) 
		-> bool { 
		common->Printf( "TODO tinyGLTF wants to write BUT WHAT! a(%s) b(%s) ", a->c_str(), b.c_str() );
		return false; 
	};

	gGLFTLoader.SetFsCallbacks(callbacks);


}
void gltfSceneEditor::Init( ) 
{
	gltfParser = new GLTF_Parser();
	gltfParser->Init();
	assetExplorer->Init();
	static auto *thisPtr = this;
#pragma region SystemCommands
	cmdSystem->AddCommand( "gltf_loadFile", []( const idCmdArgs &args ) 
		-> auto {
		if ( args.Argc( ) != 2 )
			common->Printf( "use: gltf_loadFile <file>" );
		else
			thisPtr->LoadFile( args.Argv( 1 ) );
	}, CMD_FL_SYSTEM, "Loads an gltf file", idCmdSystem::ArgCompletion_GltfName );

	cmdSystem->AddCommand( "gltf_listFiles", []( const idCmdArgs &args )
		-> auto {
		common->Printf("%i glTF files\n", thisPtr->GetLoadedFiles().Num());
		for (auto& file : thisPtr->GetLoadedFiles() )
			common->Printf( "  %-21s \n", file.c_str());
	}, CMD_FL_SYSTEM, "lists all loaded .gltf and .glb files" );

	cmdSystem->AddCommand( "gltf_listAssets", []( const idCmdArgs &args )
		-> auto {
		common->Printf("%i glTF assets\n", thisPtr->GetLoadedAssets().Num());
		int cnt = 0;
		for (auto &asset : thisPtr->GetLoadedAssets())
		{
			common->Printf(" - %s",thisPtr->GetLoadedFiles()[cnt++].c_str());
			if (!asset.asset.extras.Keys().size())
				common->Printf("  %-21s","<No MetaData>" );
			else
				for (auto& key : asset.asset.extras.Keys() )
				{
					common->Printf( "  %-21s : %-21s\n", key.c_str(),
						asset.asset.extras.Get(key).Get<std::string>().c_str() );
				}
		}
	}, CMD_FL_RENDERER, "lists all loaded gltf models" );

	cmdSystem->AddCommand( "gltf_listTextures", []( const idCmdArgs &args )
		-> auto {
		common->Printf(" - Textures (^2green ^7= data loaded)\n");
		for (auto &asset : thisPtr->GetLoadedAssets())
		{
			for (auto& img : asset.images)
				common->Printf( "  %-21s : %s%-21s\n", img.name.length() ? img.name.c_str() : "<No Image Name>",
					img.image.empty()? "^1" : "^2", img.uri.c_str());
		}
	}, CMD_FL_RENDERER, "list (loaded) textures" );

	cmdSystem->AddCommand( "gltf_listMeshes", []( const idCmdArgs &args )
		-> auto {
		common->Printf(" - Meshes" );
		int totalMeshes = 0;
		for (auto &asset : thisPtr->GetLoadedAssets())
		{
			totalMeshes += asset.meshes.size();
			for ( auto &mesh : asset.meshes )
				common->Printf( "  %-21s \n", mesh.name.length() ? mesh.name.c_str() : "<No Mesh Name>");
		}
		common->Printf( "%i glTF meshes total\n", totalMeshes );
	}, CMD_FL_RENDERER, "list loaded gltf meshes" );
	
	cmdSystem->AddCommand( "gltf_containertest", []( const idCmdArgs &args )
		-> auto {
		if ( args.Argc( ) != 2 )
			common->Printf( "use: gltf_ContainerTest <modelName>" );
		else
			thisPtr->GetRenderModel(args.Argv(1),idStr());
	}, CMD_FL_RENDERER, "test model cache" );

	cmdSystem->AddCommand( "gltf_ShowExplorer", []( const idCmdArgs &args )
		-> auto {
		assetExplorer->Show(!assetExplorer->isVisible());
	}, CMD_FL_TOOL, "Shows / Hides the Asset explorer window" );

	cmdSystem->AddCommand( "gltf_listScenes", []( const idCmdArgs &args )
		-> auto {
		common->Printf( " - Scenes" );
		int totalScenes = 0;
		for (auto &asset : thisPtr->GetLoadedAssets())
		{
			totalScenes += asset.scenes.size();
			for ( auto &scene : asset.scenes )
				common->Printf( "  %-21s \n", scene.name.length() ? scene.name.c_str() : "<No Scene name>");
		}
		common->Printf( "%i glTF scenes total\n", totalScenes );
	}, CMD_FL_TOOL, "" );
#pragma endregion

	bgfxRegisterCallback([](bgfxContext_t * context ) 
		-> auto {
		thisPtr->imDraw(context);
		thisPtr->Render(context);
	} );

}
bool gltfSceneEditor::Render( bgfxContext_t *context ) {

	//float model[16];
	//bx::mtxIdentity( model );
	//bgfx::setTransform( model );

	//bgfx::setVertexBuffer( 0, context->vbh );
	//bgfx::setIndexBuffer( context->ibh );

	//bgfx::submit( 0, context->program );
	return false;
}
bool gltfSceneEditor::imDraw( bgfxContext_t *context ) {
	auto curCtx = ImGui::GetCurrentContext();
	ImGui::SetNextWindowPos( idVec2(0,0), ImGuiCond_Once );
	if (ImGui::Begin("GLTF SCENE",&windowOpen, ImGuiWindowFlags_MenuBar))
	{
		if ( ImGui::BeginMenuBar( ) ) {
			if ( ImGui::BeginMenu( "File" ) ) {
				if ( ImGui::MenuItem( "Close" ) )
					bool open = false;

				ImGui::EndMenu( );
			}
			if ( ImGui::BeginMenu( "Windows" ) ) {
				if ( ImGui::MenuItem( "GltfExplorer Editor" ) ) {
					assetExplorer->Show(true);
				}

				ImGui::EndMenu( );
			}
			ImGui::EndMenuBar( );
		}
		{ImGui::PushID("SceneView");
		ImGui::BeginChild( "left pane", idVec2( 150, 0 ), true );
		for (auto &asset : GetLoadedAssets())
		{
			for (auto& mesh : asset.meshes )
			{
				if (ImGui::Selectable( mesh.name.c_str(),  false, ImGuiSelectableFlags_AllowDoubleClick))
				{}
			}
				//common->Printf( "  %-21s : %-21s\n", mesh.name.c_str(),
				//	mesh.extras_json_string.c_str() );
		}
		if ( ImGui::BeginPopupContextWindow( ) ) 	{//add primitive
			if ( ImGui::MenuItem( "Add Primitive" ) ) {

			}
			ImGui::EndPopup( );
		}
		ImGui::EndChild( );
		ImGui::SameLine();
		auto handle = renderModels.begin();
		if (handle.p && handle.p->textures.Num())
			ImGui::Image((void*)(intptr_t)handle.p->textures[0].handle.idx, idVec2((float)500, (float)500), idVec2(0.0f, 1.0f), idVec2(1.0f, 0.0f));

		if ( bgfx::isValid( context->rb ) )
			ImGui::Image((void*)(intptr_t) context->rb.idx, idVec2((float)context->width/4, (float)context->height/4), idVec2(0.0f, 0.0f), idVec2(1.0f, 1.0f));
		}ImGui::PopID(/*SceneView*/);
	}
	ImGui::End();
	return true;
}
int gltfSceneEditor::IsFileLoaded(const char *file )
{
	return loadedFiles.FindIndex( idStr(file) );
}
bool gltfSceneEditor::LoadFile( const char *binFile ) 
{
	common->SetRefreshOnPrint( true );
	if ( IsFileLoaded(binFile) != -1 )
	{
		if ( cvarSystem->GetCVarInteger( "fs_debug" ))
			common->DPrintf( "%s was already loaded \n", binFile );
		return true;
	}
	if (!gltfParser->Load( binFile ))
		return false;

	//std::string ext = tinygltf::GetFilePathExtension( binFile );
	//tinygltf::Model &model = loadedAssets.Alloc();
	//std::string err;
	//std::string warn;
	//bool ret = false;
	//if ( ext.compare( "glb" ) == 0 || ext.compare( "bin" ) == 0 ) {
	//	common->DPrintf( "Loading %s as a binary\n", binFile );
	//	// assume binary glTF.
	//	ret = gGLFTLoader.LoadBinaryFromFile( &model, &err, &warn, binFile );
	//} else {
	//	// assume ascii glTF.
	//	common->DPrintf( "Loading %s as a ASCII\n", binFile );
	//	ret = gGLFTLoader.LoadASCIIFromFile( &model, &err, &warn, binFile );
	//}

	//if ( !warn.empty( ) ) {
	//	common->Warning( "%s", warn.c_str( ) );
	//}

	//if ( !err.empty( ) ) {
	//	common->Error( "%s", err.c_str( ) );
	//}
	//if ( !ret ) {
	//	common->Error( "Failed to load .glTF : %s\n", binFile );
	//}
	loadedFiles.Append(	binFile );
	common->Printf( "^2Loading %s Done",binFile );
	common->SetRefreshOnPrint( false );
	return true;
}
bool gltfSceneEditor::Show( bool visible ) {
	if ( windowOpen != visible ) 	{
		windowOpen = visible;
		return true;
	}
	return false;
}

bool LoadTextureForModel( bgfxModel & model, const tinygltf::Image & img)
{
	if (!img.image.empty( ))
	{
		auto &texture = model.textures.Alloc( );
		texture.dim = idVec2( img.width, img.height );
		uint32_t tex_flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;//add point and repeat
		texture.handle = bgfx::createTexture2D( img.width, img.height, false, 1, bgfx::TextureFormat::RGBA8, tex_flags, bgfx::copy( img.image.data( ), img.width * img.height * 4 ) );
		return texture.handle.idx != -1;
	}
	return false;
}

bgfxModel * gltfSceneEditor::GetRenderModel(const idStr & meshName, const idStr & assetName )
{
	//return cached model
	int idx = modelNames.FindIndex(meshName);
	if ( idx != -1)
	{
		common->DPrintf("%s was already cached" , meshName.c_str());
		return & renderModels[idx];
	}
	
	//find mesh
	if (1 ) { }
	// load / get dummy assets
	int textureDummyIdx = IsFileLoaded( "dummies.gltf" );
	if ( textureDummyIdx < 0)
	{
		if (LoadFile ("dummies.gltf"))
		{
			textureDummyIdx = IsFileLoaded( "dummies.gltf" );
			auto & dummyAsset = GetLoadedAssets()[textureDummyIdx];
			int newIdx = modelNames.AddUnique( "textureDummy" );
			bgfxModel &textureDummy = renderModels.Alloc( );
			for (auto& img : dummyAsset.images)
			{
				if (!LoadTextureForModel( textureDummy,img))
					return false;
			}
			textureDummyIdx = newIdx;
		}
	}else
		textureDummyIdx = modelNames.FindIndex("textureDummy");

	auto & dummyTextures = renderModels[textureDummyIdx].textures;
	
	//create new 
	int newIdx = modelNames.AddUnique( meshName );
	bgfxModel& out = renderModels.Alloc();
	for (auto &asset : GetLoadedAssets())
	{
		//always add dummy textures
		for (auto & dtex : dummyTextures )
		{
			auto &texture = out.textures.Alloc();
			texture = dtex;
		}
		//Textures
		for (auto& img : asset.images)
		{
			if (!img.image.empty( ))
			{
				auto &texture = out.textures.Alloc();
				texture.dim = idVec2(img.width,img.height );
				uint32_t tex_flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;//add point and repeat
				texture.handle = bgfx::createTexture2D(img.width,img.height,false,1, bgfx::TextureFormat::RGBA8, tex_flags,bgfx::copy(img.image.data(), img.width * img.height * 4));
			}
		}
		//Materials 
		for (auto & mat :  asset.materials )
		{
			if (mat.name.empty() )
				common->FatalError("cant load unnamed gltf materials");

			int nameHash = materialIndices.GenerateKey(mat.name.c_str(),true );
			//HVG_TODO
			//NEED TO CHECK TRANSPARANCY MODE
			int materialIndex = materialIndices.First(nameHash);

			if ( materialIndex > 0 )
			{
				common->DWarning("skipping gltf material %s (%i), already loaded.",mat.name.c_str(), nameHash );
				continue;
			}

			bgfxMaterial & bgfxMaterialRef = materialList.Alloc();
			PBRMaterial & materialData = bgfxMaterialRef.material;
			materialIndices.Add( nameHash,materialList.Num()-1 );

			materialData =	PBRMaterial{
				idVec4(1.0f, 1.0f, 1.0f, 1.0f), // baseColorFactor
				idVec4(0.0f, 0.0f, 0.0f, 1.0f), // emissiveFactor
				0.5f,                     // alphaCutoff
				1.0f,                     // metallicFactor
				1.0f,                     // roughnessFactor
				out.textures[0].handle, // baseColorTexture as dummy_white
				out.textures[1].handle, // metallicRoughnessTexture as dummy_metallicRoughness;
				out.textures[2].handle, // normalTexture as dummy_normal_map;
				out.textures[0].handle, // emissiveTexture as dummy_white;
				out.textures[0].handle, // occlusionTexture as dummy_white;
			};
			TransparencyMode transparency_mode = TransparencyMode::OPAQUE_;

			auto valuesEnd = mat.values.end( );
			auto p_keyValue = mat.values.find( "baseColorTexture" );
			if ( p_keyValue != valuesEnd )         {
				materialData.baseColorTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;
			};

			p_keyValue = mat.values.find( "baseColorFactor" );
			if ( p_keyValue != valuesEnd )         {
				const auto &data = p_keyValue->second.ColorFactor( );
				materialData.baseColorFactor = idVec4(
					static_cast< float >( data[0] ),
					static_cast< float >( data[1] ),
					static_cast< float >( data[2] ),
					static_cast< float >( data[3] )
				);
			}
			p_keyValue = mat.values.find( "metallicRoughnessTexture" );
			if ( p_keyValue != valuesEnd )         {
				materialData.metallicRoughnessTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;
			}

			p_keyValue = mat.values.find( "metallicFactor" );
			if ( p_keyValue != valuesEnd )         {
				materialData.metallicFactor = static_cast< float >( p_keyValue->second.Factor( ) );
			}

			// Additional Factors
			valuesEnd = mat.additionalValues.end( );
			p_keyValue = mat.additionalValues.find( "normalTexture" );
			if ( p_keyValue != valuesEnd )         {
				materialData.normalTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;
			}

			p_keyValue = mat.additionalValues.find( "emissiveTexture" );
			if ( p_keyValue != valuesEnd )         {
				materialData.emissiveTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;

				if ( mat.additionalValues.find( "emissiveFactor" ) != valuesEnd )             {
					const auto &data = mat.additionalValues.at( "emissiveFactor" ).ColorFactor( );
					materialData.emissiveFactor = idVec4(
						static_cast< float >( data[0] ),
						static_cast< float >( data[1] ),
						static_cast< float >( data[2] ),
						1.0f );
				}
			};

			p_keyValue = mat.additionalValues.find( "occlusionTexture" );
			if ( p_keyValue != valuesEnd )         {
				materialData.occlusionTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;
			}

			p_keyValue = mat.additionalValues.find( "metallicRoughnessTexture" );
			if ( p_keyValue != valuesEnd )         {
				materialData.metallicRoughnessTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;
			}

			p_keyValue = mat.additionalValues.find( "alphaMode" );
			if ( p_keyValue != valuesEnd )         {
				if ( p_keyValue->second.string_value == "BLEND" )             {
					transparency_mode = TransparencyMode::BLENDED;
				}             else if ( p_keyValue->second.string_value == "MASK" )             {
					transparency_mode = TransparencyMode::MASKED;
				}
			}

			p_keyValue = mat.additionalValues.find( "alphaCutoff" );
			if ( p_keyValue != valuesEnd )         {
				materialData.alphaCutoff = static_cast< float >( p_keyValue->second.Factor( ) );
			}

			bgfxMaterialRef.TransparencyMode = transparency_mode;
		}
	}

	return &out;
}


gltfAssetExplorer::gltfAssetExplorer( )  
{
	guiVisible = false;
	selectedFileHash = 0;
	selectedImage = nullptr;
	selectedMesh = nullptr;
} 
gltfAssetExplorer::~gltfAssetExplorer( ) 
{
}
void gltfAssetExplorer::Init()
{
	static auto * thisPtr = this;
	bgfxRegisterCallback([](bgfxContext_t * context ) 
		-> auto {
		thisPtr->imDraw(context);
		thisPtr->Render(context);
	} );
}
bool gltfAssetExplorer::Show(bool visible )
{
	if (guiVisible != visible )
	{
		guiVisible = visible;
		return true;
	}
	return false;
}
bool gltfAssetExplorer::Render( bgfxContext_t *context ) 
{
	static bgfx::UniformHandle  g_AttribLocationTex =
		bgfx::createUniform( "colorUniformHandle", bgfx::UniformType::Sampler );
	if (selectedMesh != nullptr )
	{
		float modelTransform[16];
		bx::mtxIdentity( modelTransform );
		//bx::mtxMul( tmp, modelScale, xtmp );
		//bx::mtxMul( modelTransform, tmp, modelTranslation );
	    bgfx::setTransform( modelTransform );
		for ( auto prim : selectedMesh->primitives )
		{
			float modelScale[16];
			float tmp[16];
			bx::mtxScale( modelScale, 0.8f + idMath::ClampFloat( 0.2f, 10.0f, ( abs( sin( 0.001f * com_frameTime ) ) ) ) );// sin( com_frameTime ) );

			idVec3 dir = idVec3( -1, -1, 0 );
			dir.Normalize( );
			idRotation modelRot( idVec3( 0, 0, 0 ), dir, RAD2DEG( abs( 0.001f * com_frameTime ) ) );
			idRotation modelRot2( idVec3( 0, 0, 0 ), idVec3( 0, -1, 0 ), RAD2DEG( abs( 0.001f * com_frameTime ) ) );
			idMat4 rotmat = modelRot.ToMat4( );
			float * modelTransform = rotmat.ToFloatPtr( );
			bx::mtxMul( tmp, modelScale, modelTransform );
			bgfx::setTransform( tmp );
			bgfx::setTexture( 0, g_AttribLocationTex, selectedImage->bgfxTexture.handle );
			bgfx::setVertexBuffer( 0, prim->vertexBufferHandle );
			bgfx::setIndexBuffer( prim->indexBufferHandle );
			bgfx::submit( 1, context->program );
		}

	    
	}
	return false;
}
bool gltfAssetExplorer::imDraw( bgfxContext_t *context ) 
{
	if (!guiVisible)
		return guiVisible;

	const auto & fileList = sceneEditor->GetLoadedFiles();
	//pre-select first item.
	if ( selectedFileHash==0 && fileList.Num())
		selectedFileHash = idStr::Hash(fileList.begin().p->c_str());
	
	auto curCtx = ImGui::GetCurrentContext();
	ImGui::SetNextWindowPos( idVec2(0,0), ImGuiCond_Once );
	if (ImGui::Begin("Asset Explorer",&guiVisible, ImGuiWindowFlags_MenuBar))
	{
		if ( ImGui::BeginMenuBar( ) ) {
			if ( ImGui::BeginMenu( "File" ) ) {
				if ( ImGui::MenuItem( "Close" ) )
					guiVisible = false;

				ImGui::EndMenu( );
			}
			
			ImGui::EndMenuBar( );
		}
		{ImGui::PushID("SceneView");
		ImGui::BeginChild( "left pane", idVec2( 250, 0 ), true, ImGuiWindowFlags_HorizontalScrollbar );
		for (auto &file : sceneEditor->GetLoadedFiles())
		{
			int currentFileHash = idStr::Hash(file);
			ImGui::PushID( currentFileHash );
			ImGui::PushStyleColor( ImGuiCol_Button, IM_COL32( 255, 0, 0, 100 ) );
			if ( ImGui::SmallButton( "X" ) )
				common->Printf("Unload me %s", file.c_str() );
			ImGui::SameLine();
			bool selected = selectedFileHash == currentFileHash;
			if (ImGui::Selectable( file.c_str(), selected,ImGuiSelectableFlags_AllowDoubleClick))
			{
				selectedFileHash = currentFileHash;
				common->Printf("Selected : %s",file.c_str() );
			}
			if( selected )
				DrawImAssetTree( );
			ImGui::PopStyleColor( );
			ImGui::PopID();//file item
		}
		if ( ImGui::BeginPopupContextWindow( ) ) 	
		{//add primitive
			if ( ImGui::MenuItem( "Load" ) ) {

			}
			if ( ImGui::MenuItem( "Unload" ) ) {

			}
			ImGui::EndPopup( );
		}
		ImGui::EndChild( );
		ImGui::SameLine();
		if ( selectedImage != nullptr )
			ImGui::Image((void*)(intptr_t) selectedImage->bgfxTexture.handle.idx, idVec2((float)context->width/4, (float)context->height/4), idVec2(0.0f, 0.0f), idVec2(1.0f, 1.0f));
		}ImGui::PopID(/*SceneView*/);
	}
	ImGui::End();
	return false;
}
void gltfAssetExplorer::DrawImAssetTree( )
{
	static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiSelectableFlags_AllowDoubleClick;
	int assetCnt = 0;
	for (auto & data : gltfData::DataList() )
	{
		if (selectedFileHash != data->FileNameHash())
			continue;

		int imageCount = 0;
		auto & images= data->ImageList( );

		int meshCount = 0;
		auto & meshList = data->MeshList( );

		int assetID = idStr::Hash( ( idStr( "Asset" ) + assetCnt++ ).c_str( ) );
		if ( images.Num() )
		{
			const static int imagesHash = idStr::Hash("images");
			bool drawImages = ImGui::TreeNodeEx((void*)(intptr_t)( imagesHash ), base_flags, "Images %i",images.Num());
			if ( drawImages )
			{
				for (auto image : images )
				{
					int fileHash = data->FileNameHash( );
					if (selectedFileHash == fileHash)
					{
						idStr name = image->name.IsEmpty() ? image->uri.IsEmpty() ? data->FileName() : image->uri : image->name;
						bool selected = selectedImage == image;
						{ImGui::PushID(( idStr( "image" ) + imageCount ).c_str( ));
						if(ImGui::Selectable( name.c_str( ), selected, ImGuiSelectableFlags_AllowDoubleClick ))
						{
							selectedImage = image;
							selectedMesh = nullptr;
						}
						ImGui::PopID(/*image*/ );}
					}
				}
				ImGui::TreePop();
			}
		}
		if ( meshList.Num() )
		{
			const static int meshesHash = idStr::Hash( "meshes" );
			bool drawMeshes = ImGui::TreeNodeEx((void*)(intptr_t)( meshesHash ), base_flags, "Meshes %i", meshList.Num());
			if ( drawMeshes )
			{
				for (auto mesh : meshList )
				{
					int fileHash = data->FileNameHash( );
					if (selectedFileHash == fileHash)
					{
						int meshID = idStr::Hash( ( idStr( "mesh" ) + meshCount++ ).c_str( ) );
						idStr name = mesh->name.IsEmpty() ? data->FileName() : mesh->name;
						bool selected = selectedMesh == mesh;
						{ImGui::PushID(( idStr( "image" ) + imageCount ).c_str( ));
						if(ImGui::Selectable( name.c_str( ), selected, ImGuiSelectableFlags_AllowDoubleClick ))
						{
							selectedMesh = mesh;
							//selectedImage = nullptr;
						}
						ImGui::PopID(/*image*/ );}
					}
				}
				ImGui::TreePop();
			}
		}
	}
}
