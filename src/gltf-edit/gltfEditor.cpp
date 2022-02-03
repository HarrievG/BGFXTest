#include "gltfEditor.h"
//#define TINYGLTF_IMPLEMENTATION
//#define TINYGLTF_USE_CPP14
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
//// no need for it, it should go,  but doenst compile ootb..
////#define TINYGLTF_NO_FS
//#include "tiny_gltf.h"
////////////////////////

#include "imgui.h"
#include "CVarSystem.h"
#include "Common.h"
#include "FileSystem.h"
#include "CVarSystem.h"
#include "bgfx/bgfx.h"
#include "bimg/bimg.h"
#include "gltfParser.h"
#include <ImGuizmo.h>

//tinygltf::TinyGLTF gGLFTLoader;

static gltfSceneEditor localSceneEditor;
gltfSceneEditor *sceneEditor = &localSceneEditor;

static gltfAssetExplorer localAssetExplorer;
gltfAssetExplorer *assetExplorer = &localAssetExplorer;

static GLTF_Parser * gltfParser;

idCVar r_gltfEditRenderWidth( "r_SceneEditRenderWidth", "1024", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, "gltfEditor render view width. set r_mode to -1 to use backbuffer size" );
idCVar r_gltfEditRenderHeight( "r_SceneEditRenderHeight", "768", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, "gltfEditor render view height.set r_mode to -1 to use backbuffer size" );

void gltfSceneEditor::Shutdown( )
{
	delete gltfParser;
}
gltfSceneEditor::gltfSceneEditor( )
	: windowOpen(false)
{
}
void gltfSceneEditor::Init( ) 
{
	gltfParser = new GLTF_Parser();
	gltfParser->Init();
	assetExplorer->Init();
	static auto *thisPtr = this;
	selectedScene = nullptr;
	currentData	 = nullptr;
	selectedNode = nullptr;
	currentCamera = nullptr;
	selectedCameraId = -1;

	renderTarget.width = r_gltfEditRenderWidth.GetInteger( );
	renderTarget.height = r_gltfEditRenderHeight.GetInteger( );
	renderTarget.viewId = 20;

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

	cmdSystem->AddCommand( "gltf_listTextures", []( const idCmdArgs &args )
		-> auto {
		common->Printf(" - Textures [imageName : bgfxTextureHandle] (^2green ^7= data loaded)\n");
		auto &dataList = gltfData::DataList( );
		for ( auto &data : dataList )
		{
			auto &imgList = data->ImageList( );
			for (auto& img : imgList )
				common->Printf( "%s :%s %i\n", !img->name.IsEmpty() ? img->name.c_str() : "<No Image Name>",
					img->bgfxTexture.handle.idx == bgfx::kInvalidHandle ? "^1" : "^2", img->bgfxTexture.handle.idx );
		}
	}, CMD_FL_RENDERER, "list (loaded) textures" );

	cmdSystem->AddCommand( "gltf_listMeshes", []( const idCmdArgs &args )
		-> auto {
		common->Printf(" - Meshes" );
		int totalMeshes = 0;
		auto &dataList = gltfData::DataList( );
		for ( auto &data : dataList )
		{
			auto &meshList = data->MeshList( );
			totalMeshes += meshList.Num();
			for ( auto &mesh : meshList )
				common->Printf( "  %-21s \n", !mesh->name.IsEmpty() ? mesh->name.c_str() : "<No Mesh Name>");
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
		auto &dataList = gltfData::DataList( );
		for (auto &data : dataList )
		{
			auto & scenelist = data->SceneList();
			totalScenes += scenelist.Num();
			for ( auto &scene : scenelist )
				common->Printf( "  %-21s \n", !scene->name.IsEmpty() ? scene->name.c_str() : "<No Scene name>");
		}
		common->Printf( "%i glTF scenes total\n", totalScenes );
	}, CMD_FL_TOOL, "" );
#pragma endregion

	bgfxRegisterCallback([](bgfxContext_t * context ) 
		-> auto {
		thisPtr->imDraw(context);
		thisPtr->Render(context);
	} );

	float fov = 27.f;
	float camYAngle = 165.f / 180.f * 3.14159f;
	float camXAngle = 32.f / 180.f * 3.14159f;
	bx::Vec3 eye = { cosf( camYAngle ) * cosf( camXAngle ) * 10, sinf( camXAngle ) * 10, sinf( camYAngle ) * cosf( camXAngle ) * 10 };
	bx::Vec3 at = { 0.f, 0.f, 0.f };
	bx::Vec3 up = { 0.f, 1.f, 0.f };
	bx::mtxLookAt( cameraView.ToFloatPtr( ), eye, at, up, bx::Handness::Left );
}
void gltfSceneEditor::RenderSceneNode( bgfxContext_t *context, gltfNode *node, idMat4 trans, const idList<gltfNode *> &nodeList )
{
	idMat4 mat;
	gltfData::ResolveNodeMatrix( node, &mat);
	idMat4 curTrans = trans * node->matrix ;
	static bgfx::UniformHandle  g_AttribLocationTex = bgfx::createUniform( "colorUniformHandle", bgfx::UniformType::Sampler );
	if ( node->mesh != -1 ) 		
	{
		for ( auto prim : currentData->MeshList()[node->mesh]->primitives )
		{
			bgfx::setTransform( curTrans.Transpose().ToFloatPtr() );

			//bgfx::setTexture( 0, g_AttribLocationTex, selectedImage->bgfxTexture.handle );

			bgfx::setVertexBuffer( 0, prim->vertexBufferHandle );
			bgfx::setIndexBuffer( prim->indexBufferHandle );
			bgfx::submit( renderTarget.viewId, context->program );
		}
	}

	if ( selectedCameraId != -1 && node->camera == selectedCameraId )
	{
		idVec3 idup = idVec3(curTrans[0][1],curTrans[1][1],curTrans[2][1]);
		idVec3 forward = idVec3(- curTrans[0][2],- curTrans[1][2], -curTrans[2][2] );
		idVec3 pos = idVec3(curTrans[0][3],curTrans[1][3],curTrans[2][3] );

		bx::Vec3 at = bx::Vec3(pos.x + forward.x, pos.y +forward.y ,pos.z+forward.z );
		bx::Vec3 eye = bx::Vec3(pos.x,pos.y,pos.z);
		bx::Vec3 up = bx::Vec3(idup.x,idup.y,idup.z );
		bx::mtxLookAt( cameraView.ToFloatPtr( ), eye, at, up ,bx::Handness::Right );
	}

	for ( auto &child : node->children )
		RenderSceneNode( context, nodeList[child], curTrans, nodeList );
}
bool gltfSceneEditor::Render( bgfxContext_t *context ) 
{
	if ( !bgfx::isValid( renderTarget.fbh ) )
	{
		//load default assets
		LoadFile( "blender/SceneExplorerAssets.glb" );
		bgfxCreateMrtTarget( renderTarget, "SceneEditorView" );
		editorData =  gltfData::Data( "blender/SceneExplorerAssets.glb" );

		//float xfov = 90;
		//float yfov = 2 * atan( ( float ) context->height / context->width ) * idMath::M_RAD2DEG;

		//float	xmin, xmax, ymin, ymax;
		//float	width, height;
		//float	zNear;

		//float	*xprojectionMatrix =  cameraProjection.ToFloatPtr( );
		////
		//// set up projection matrix
		////
		//zNear = 0.01f;

		//ymax = zNear * tan( yfov * idMath::PI / 360.0f );
		//ymin = -ymax;

		//xmax = zNear * tan( xfov * idMath::PI / 360.0f );
		//xmin = -xmax;

		//width = xmax - xmin;
		//height = ymax - ymin;

		//xprojectionMatrix[0] = 2 * zNear / width;
		//xprojectionMatrix[4] = 0;
		//xprojectionMatrix[8] = ( xmax + xmin ) / width;	// normally 0
		//xprojectionMatrix[12] = 0;
		//
		//xprojectionMatrix[1] = 0;
		//xprojectionMatrix[5] = 2 * zNear / height;
		//xprojectionMatrix[9] = ( ymax + ymin ) / height;	// normally 0
		//xprojectionMatrix[13] = 0;

		//// this is the far-plane-at-infinity formulation
		//xprojectionMatrix[2] = 0;
		//xprojectionMatrix[6] = 0;
		//xprojectionMatrix[10] = -1;
		//xprojectionMatrix[14] = -2 * zNear;
		//
		//xprojectionMatrix[3] = 0;
		//xprojectionMatrix[7] = 0;
		//xprojectionMatrix[11] = -1;
		//xprojectionMatrix[15] = 0;

		cameraView = mat4_identity;
		pos = idVec3(0.f,0,0.f );
		scale = idVec3( 1.f, 1, 1.f );
		currentData = editorData;
		anglesX = idAngles(0,0,0 );
		anglesQ = anglesX.ToQuat();
		anglesX.Set(0,0,0);
	}
	idMat4 scaleMat = idMat4(
		scale.x, 0, 0, 0,
		0, scale.y, 0, 0,
		0, 0, scale.z, 0,
		0, 0, 0, 1
	);

	bgfx::touch( renderTarget.viewId );

	if (selectedScene && currentData)
	{
		if (selectedCameraId != -1 )
		{
			gltfCamera_Perspective &sceneCam = currentData->CameraList( )[selectedCameraId]->perspective;
			bx::mtxProj( cameraProjection.ToFloatPtr( ), RAD2DEG( sceneCam.yfov ), sceneCam.aspectRatio, sceneCam.znear, sceneCam.zfar, bgfx::getCaps( )->homogeneousDepth, bx::Handness::Right );
		}

		if (!currentData->CameraList().Num() )
		{
			idVec3 tmp = idVec3( anglesX.ToFloatPtr( )[0], anglesX.ToFloatPtr( )[1], anglesX.ToFloatPtr( )[2] );
			idAngles tmpAngles = idAngles( tmp );

			cameraView = idMat4( mat3_identity, pos ) * tmpAngles.ToMat4( ).Transpose() * scaleMat;
		}

		auto &nodeList = currentData->NodeList( ); 
		idMat4 mat;
		auto &scenes = currentData->SceneList( );
		for ( auto &scene : scenes )
			for ( auto &node : scene->nodes)
			{
				idMat4 mat = mat4_identity;
				RenderSceneNode(context, nodeList[node], mat, nodeList);
			}
	}

	bgfx::setViewTransform( renderTarget.viewId, cameraView.ToFloatPtr( ), cameraProjection.ToFloatPtr( ) );

	if ( bgfx::isValid( renderTarget.rb ) )
		bgfx::blit( 100 - renderTarget.viewId, renderTarget.rb, 0, 0, renderTarget.fbTextureHandles[0] );
	return false;
}
bool gltfSceneEditor::imDraw( bgfxContext_t *context ) {
	auto curCtx = ImGui::GetCurrentContext();
	ImGui::SetNextWindowPos( idVec2(0,0), ImGuiCond_FirstUseEver );

	//ImGui::Begin( "Gizmo", 0, ImGuiWindowFlags_NoMove );
	ImGuiIO &io = ImGui::GetIO( );

	static idVec2 localMousePos;
	static const idVec2 viewGizmoSize = idVec2( 125.f, 125.f );
	static idVec2 localWindowPos,localWindowSize;

	idVec2 viewGizmoPos; 

	static bool gizmoHover = false;
	static bool scrolling = false;
	static int localScrolY = 0;
	ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
	if ( gizmoHover )
		flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

	if (ImGui::Begin("GLTF SCENE",&sceneViewOpen, flags ))
	{
		localWindowPos = ImGui::GetWindowPos( );
		localWindowSize = ImGui::GetWindowSize();
		localMousePos = idVec2( io.MousePos.x - ImGui::GetWindowPos( ).x, io.MousePos.y - ImGui::GetWindowPos( ).y );
		viewGizmoPos = ImVec2( localWindowPos.x + localWindowSize.x - ( viewGizmoSize.x + 20 ), localWindowPos.y + 20 );

		if (!scrolling && io.MouseDown[0])
			scrolling = localScrolY != ImGui::GetScrollY( );
		else if(io.MouseReleased[0])
			scrolling = false;

		localScrolY = ImGui::GetScrollY( );

		gizmoHover = localMousePos.x > (localWindowSize.x - viewGizmoSize.x - 50);
		gizmoHover &= localMousePos.y < (viewGizmoSize.y + 50);
		gizmoHover |= ImGui::IsItemActive( );
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
			if ( ImGui::BeginMenu( "Scene" ) ) {
				if ( ImGui::BeginMenu( "Cameras" ) ) {
					if (currentData && selectedScene ) {
						int camCount = 0;
						idList<gltfCamera*> cams = currentData->CameraList( );
						for (auto & camera : cams ) {
							idStr camName;
							if (!camera->name.IsEmpty( ))
								camName = camera->name.c_str( );
							else
								camName = currentData->FileName() + idStr(camCount);
							ImGui::PushID( camCount );
							if (ImGui::MenuItem(camName.c_str())) {
								currentCamera = cams[camCount];
								selectedCameraId = camCount;
							}
							ImGui::PopID(  );
							camCount++;
						}
					}
					ImGui::EndMenu( );
				}
				ImGui::EndMenu( );
			}
			ImGui::EndMenuBar( );
		}
		//fixme DrawCameraInfo( currentCamera );
		DrawNodeInfo( selectedNode );
		ImGui::Image( ( void * ) ( intptr_t ) renderTarget.rb.idx, idVec2( ( float ) renderTarget.width, ( float ) renderTarget.height ), idVec2( 0.0f, 0.0f ), idVec2( 1.0f, 1.0f ) );
	}
	ImGui::End();
	DrawSceneList();

	return true;
}
void gltfSceneEditor::DrawCameraInfo( gltfCamera *camera )
{
	if ( !currentData || !currentCamera ) 
		return;

	static bool p_open = true;
	static int corner = 1;
	ImGuiIO &io = ImGui::GetIO( );
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	if ( corner != -1 )     {
		const float PAD = 50.0f;
		const ImGuiViewport *viewport = ImGui::GetMainViewport( );

		ImVec2 work_pos = ImGui::GetWindowPos( );
		ImVec2 work_size = ImGui::GetWindowSize( );

		ImVec2 window_pos, window_pos_pivot;
		window_pos.x = ( corner & 1 ) ? ( work_pos.x + work_size.x - PAD ) : ( work_pos.x + PAD );
		window_pos.y = ( corner & 2 ) ? ( work_pos.y + work_size.y - PAD ) : ( work_pos.y + PAD );
		window_pos_pivot.x = ( corner & 1 ) ? 1.0f : 0.0f;
		window_pos_pivot.y = ( corner & 2 ) ? 1.0f : 0.0f;
		ImGui::SetNextWindowPos( window_pos, ImGuiCond_Always, window_pos_pivot );
		window_flags |= ImGuiWindowFlags_NoMove;
	}
	ImGui::SetNextWindowBgAlpha( 0.35f ); // Transparent background
	if ( ImGui::Begin( "Camera info", &p_open, window_flags ) ) {
	
		gltfCameraNodePtrs res = currentData->GetCameraNodes( currentCamera );

		if ( io.MouseDown[1] ) {

			res.translationNode->rotation *= idAngles(0, io.MouseDelta.x * ( com_frameTime / 100000.0f ), io.MouseDelta.y *  ( com_frameTime / 100000.0f )).ToQuat();


			idVec3 dir = idVec3( cameraView[2][0], cameraView[2][1], cameraView[2][2] );

			//idQuat dir = res.orientationNode != nullptr ? res.translationNode->rotation + res.orientationNode->rotation : res.translationNode->rotation;

			if ( ImGui::IsKeyDown( SDL_SCANCODE_W ) ) {
				res.translationNode->translation -= dir *  ( com_frameTime / 100000.0f );
			}
			if ( ImGui::IsKeyDown( SDL_SCANCODE_S ) ) {
				res.translationNode->translation += dir *  ( com_frameTime / 100000.0f );
			}
			if ( ImGui::IsKeyDown( SDL_SCANCODE_A ) ) {
				res.translationNode->rotation *= idAngles( ( com_frameTime / 100000.0f ),0, 0 ).ToQuat( );
			}
			if ( ImGui::IsKeyDown( SDL_SCANCODE_D ) ) {
				res.translationNode->rotation *= idAngles( -( com_frameTime / 100000.0f ),0, 0).ToQuat( );
			}
			res.translationNode->dirty = true;
		}

		ImGui::Text( "Camera" );
		if ( res.translationNode != nullptr && !currentCamera->name.IsEmpty( ) )
		{
			ImGui::SameLine();
			ImGui::Text( ": %s", currentCamera->name.c_str());
		}
		ImGui::Separator( );
		if ( res.translationNode != nullptr ) {
			ImGui::Text( "Position" );
			ImGui::DragFloat3( "##campos", res.translationNode->translation.ToFloatPtr( ) );
		}
		ImGui::Separator( );

		if ( ImGui::BeginPopupContextWindow( ) )         {
			if ( ImGui::MenuItem( "Custom", NULL, corner == -1 ) ) corner = -1;
			if ( ImGui::MenuItem( "Top-left", NULL, corner == 0 ) ) corner = 0;
			if ( ImGui::MenuItem( "Top-right", NULL, corner == 1 ) ) corner = 1;
			if ( ImGui::MenuItem( "Bottom-left", NULL, corner == 2 ) ) corner = 2;
			if ( ImGui::MenuItem( "Bottom-right", NULL, corner == 3 ) ) corner = 3;
			if ( p_open && ImGui::MenuItem( "Close" ) ) p_open = false;
			ImGui::EndPopup( );
		}
	}
	ImGui::End( );
}
void gltfSceneEditor::DrawNodeInfo( gltfNode* node )
{
	if ( !currentData || !node )
		return;
	static bool p_open = true;
	static int corner = 1;
	ImGuiIO &io = ImGui::GetIO( );
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	if ( corner != -1 ) {
		const float PAD = 50.0f;
		const ImGuiViewport *viewport = ImGui::GetMainViewport( );

		ImVec2 work_pos = ImGui::GetWindowPos( );
		ImVec2 work_size = ImGui::GetWindowSize( );

		ImVec2 window_pos, window_pos_pivot;
		window_pos.x = ( corner & 1 ) ? ( work_pos.x + work_size.x - PAD ) : ( work_pos.x + PAD );
		window_pos.y = ( corner & 2 ) ? ( work_pos.y + work_size.y - PAD ) : ( work_pos.y + PAD );
		window_pos_pivot.x = ( corner & 1 ) ? 1.0f : 0.0f;
		window_pos_pivot.y = ( corner & 2 ) ? 1.0f : 0.0f;
		ImGui::SetNextWindowPos( window_pos, ImGuiCond_Always, window_pos_pivot );
		window_flags |= ImGuiWindowFlags_NoMove;
	}
	ImGui::Separator( );
	ImGui::SetNextWindowBgAlpha( 0.35f ); // Transparent background
	if ( ImGui::Begin( "Camera info", &p_open, window_flags ) ) {

		ImGui::Text( "Node" );
		if ( node != nullptr && !node->name.IsEmpty( ) ) 		{
			ImGui::SameLine( );
			ImGui::Text( ": %s", node->name.c_str( ) );
		}
		ImGui::Separator( );
		if ( node != nullptr ) {
			bool refresh = false;
			ImGui::Text( "Position (vec3)" );
			refresh |= ImGui::DragFloat3( "##nodepas", node->translation.ToFloatPtr( ));
			idAngles ang = node->rotation.ToMat3().ToAngles();
			ImGui::Text( "Rotation (angle %)" );
			if ( ImGui::DragFloat3( "##noderot", ang.ToFloatPtr( ) ) )
			{
				refresh = true;
				node->rotation = ang.ToQuat();
			}
			ImGui::Text( "Scale (vec3)" );
			refresh |= ImGui::DragFloat3( "##nodescale", node->scale.ToFloatPtr( ) );
			node->dirty |= refresh;
		}

		ImGui::Separator( );
		if ( ImGui::IsMousePosValid( ) )
			ImGui::Text( "Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y );
		else
			ImGui::Text( "Mouse Position: <invalid>" );
	}
	ImGui::End( );
}
void gltfSceneEditor::DrawSceneList()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
	if ( ImGui::Begin( "GLTF SCENE LIST", &windowOpen, flags ) )
	{
		{ImGui::PushID( "SceneView" );
			static const idVec2 left_pane_size = idVec2( 250, 0 );

			int sceneCount = 0;
			auto &dataList = gltfData::DataList( );
			auto &imStyle = ImGui::GetStyle( );
			ImGui::SetNextItemWidth( ImGui::GetWindowSize().x - ( imStyle.WindowPadding.x * 2 ) );
			ImGui::PushID( "##" );
			if ( ImGui::BeginCombo( "##scenes", selectedScene ? selectedScene->name.c_str( ) : "<Scene>" ) ) 
			{
				int dataCount = 0;
				for ( auto &data : dataList )
				{
					ImGui::PushID( dataCount++ );
					int sceneCount = 0;
					auto &scenes = data->SceneList( );
					for ( auto &scene : scenes )
					{
						bool selected = selectedScene == scene;
						ImGui::PushID( sceneCount++ );
						if ( ImGui::Selectable( scene->name.Length( ) ? scene->name.c_str( ) : data->FileName( ).c_str( ), selected ) )
						{
							currentData = data;
							selectedScene = scene;
							selectedCameraId = -1;
						}
						ImGui::PopID( );
					}
					ImGui::PopID( );
				}
				ImGui::EndCombo( );
			}
			ImGui::PopID( );
			//draw scene nodes
			if ( selectedScene && currentData ) 		{
				auto &nodes = currentData->NodeList( );
				for ( auto &child : selectedScene->nodes )
					DrawSceneNode( nodes[child], nodes );
			}
			if ( ImGui::BeginPopupContextWindow( ) ) {//add primitive
				if ( ImGui::MenuItem( "Add Node" ) ) {

				}
				ImGui::EndPopup( );
			}
		}ImGui::PopID(/*SceneView*/ );
	}
	ImGui::End();
}
bool gltfSceneEditor::DrawSceneNode( gltfNode *node,const  idList<gltfNode *> &nodeList )
{
	//nodeList.Remove(node);
	if ( node->children.Num( ) ) 
	{
		ImGui::PushID( "##" );
		bool opened = ImGui::TreeNode((void*)(intptr_t) node,"");
		ImGui::SameLine( );
		if ( ImGui::Selectable( node->name.Length( ) ? node->name.c_str( ) : "<Node>", selectedNode == node, ImGuiSelectableFlags_AllowDoubleClick ) )
			selectedNode = node;
		if ( opened )
		{
			for ( auto &child : node->children )
				DrawSceneNode( nodeList[child], nodeList );
			ImGui::TreePop( );
		}
		ImGui::PopID( );
	}else 
	{
		ImGui::PushID( "##" );
		ImGui::Indent( ImGui::GetTreeNodeToLabelSpacing( ) );
		if ( ImGui::Selectable( node->name.Length( ) ? node->name.c_str( ) : "<Node>", selectedNode == node, ImGuiSelectableFlags_AllowDoubleClick ) ) 
			selectedNode = node;
		ImGui::Unindent( ImGui::GetTreeNodeToLabelSpacing( ) );
		ImGui::PopID( );
	}

	return false;
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

bgfxModel * gltfSceneEditor::GetRenderModel(const idStr & meshName, const idStr & assetName )
{
	////return cached model
	//int idx = modelNames.FindIndex(meshName);
	//if ( idx != -1)
	//{
	//	common->DPrintf("%s was already cached" , meshName.c_str());
	//	return & renderModels[idx];
	//}
	//
	////find mesh
	//if (1 ) { }
	//// load / get dummy assets
	//int textureDummyIdx = IsFileLoaded( "dummies.gltf" );
	//if ( textureDummyIdx < 0)
	//{
	//	if (LoadFile ("dummies.gltf"))
	//	{
	//		textureDummyIdx = IsFileLoaded( "dummies.gltf" );
	//		//auto & dummyAsset = GetLoadedAssets()[textureDummyIdx];
	//		int newIdx = modelNames.AddUnique( "textureDummy" );
	//		bgfxModel &textureDummy = renderModels.Alloc( );
	//		for (auto& img : dummyAsset.images)
	//		{
	//			if (!LoadTextureForModel( textureDummy,img))
	//				return false;
	//		}
	//		textureDummyIdx = newIdx;
	//	}
	//}else
	//	textureDummyIdx = modelNames.FindIndex("textureDummy");

	//auto & dummyTextures = renderModels[textureDummyIdx].textures;
	//
	////create new 
	//int newIdx = modelNames.AddUnique( meshName );
	bgfxModel& out = renderModels.Alloc();
	//for (auto &asset : GetLoadedAssets())
	//{
	//	//always add dummy textures
	//	for (auto & dtex : dummyTextures )
	//	{
	//		auto &texture = out.textures.Alloc();
	//		texture = dtex;
	//	}
	//	//Textures
	//	for (auto& img : asset.images)
	//	{
	//		if (!img.image.empty( ))
	//		{
	//			auto &texture = out.textures.Alloc();
	//			texture.dim = idVec2(img.width,img.height );
	//			uint32_t tex_flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;//add point and repeat
	//			texture.handle = bgfx::createTexture2D(img.width,img.height,false,1, bgfx::TextureFormat::RGBA8, tex_flags,bgfx::copy(img.image.data(), img.width * img.height * 4));
	//		}
	//	}
	//	//Materials 
	//	for (auto & mat :  asset.materials )
	//	{
	//		if (mat.name.empty() )
	//			common->FatalError("cant load unnamed gltf materials");

	//		int nameHash = materialIndices.GenerateKey(mat.name.c_str(),true );
	//		//HVG_TODO
	//		//NEED TO CHECK TRANSPARANCY MODE
	//		int materialIndex = materialIndices.First(nameHash);

	//		if ( materialIndex > 0 )
	//		{
	//			common->DWarning("skipping gltf material %s (%i), already loaded.",mat.name.c_str(), nameHash );
	//			continue;
	//		}

	//		bgfxMaterial & bgfxMaterialRef = materialList.Alloc();
	//		PBRMaterial & materialData = bgfxMaterialRef.material;
	//		materialIndices.Add( nameHash,materialList.Num()-1 );

	//		materialData =	PBRMaterial{
	//			idVec4(1.0f, 1.0f, 1.0f, 1.0f), // baseColorFactor
	//			idVec4(0.0f, 0.0f, 0.0f, 1.0f), // emissiveFactor
	//			0.5f,                     // alphaCutoff
	//			1.0f,                     // metallicFactor
	//			1.0f,                     // roughnessFactor
	//			out.textures[0].handle, // baseColorTexture as dummy_white
	//			out.textures[1].handle, // metallicRoughnessTexture as dummy_metallicRoughness;
	//			out.textures[2].handle, // normalTexture as dummy_normal_map;
	//			out.textures[0].handle, // emissiveTexture as dummy_white;
	//			out.textures[0].handle, // occlusionTexture as dummy_white;
	//		};
	//		TransparencyMode transparency_mode = TransparencyMode::OPAQUE_;

	//		auto valuesEnd = mat.values.end( );
	//		auto p_keyValue = mat.values.find( "baseColorTexture" );
	//		if ( p_keyValue != valuesEnd )         {
	//			materialData.baseColorTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;
	//		};

	//		p_keyValue = mat.values.find( "baseColorFactor" );
	//		if ( p_keyValue != valuesEnd )         {
	//			const auto &data = p_keyValue->second.ColorFactor( );
	//			materialData.baseColorFactor = idVec4(
	//				static_cast< float >( data[0] ),
	//				static_cast< float >( data[1] ),
	//				static_cast< float >( data[2] ),
	//				static_cast< float >( data[3] )
	//			);
	//		}
	//		p_keyValue = mat.values.find( "metallicRoughnessTexture" );
	//		if ( p_keyValue != valuesEnd )         {
	//			materialData.metallicRoughnessTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;
	//		}

	//		p_keyValue = mat.values.find( "metallicFactor" );
	//		if ( p_keyValue != valuesEnd )         {
	//			materialData.metallicFactor = static_cast< float >( p_keyValue->second.Factor( ) );
	//		}

	//		// Additional Factors
	//		valuesEnd = mat.additionalValues.end( );
	//		p_keyValue = mat.additionalValues.find( "normalTexture" );
	//		if ( p_keyValue != valuesEnd )         {
	//			materialData.normalTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;
	//		}

	//		p_keyValue = mat.additionalValues.find( "emissiveTexture" );
	//		if ( p_keyValue != valuesEnd )         {
	//			materialData.emissiveTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;

	//			if ( mat.additionalValues.find( "emissiveFactor" ) != valuesEnd )             {
	//				const auto &data = mat.additionalValues.at( "emissiveFactor" ).ColorFactor( );
	//				materialData.emissiveFactor = idVec4(
	//					static_cast< float >( data[0] ),
	//					static_cast< float >( data[1] ),
	//					static_cast< float >( data[2] ),
	//					1.0f );
	//			}
	//		};

	//		p_keyValue = mat.additionalValues.find( "occlusionTexture" );
	//		if ( p_keyValue != valuesEnd )         {
	//			materialData.occlusionTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;
	//		}

	//		p_keyValue = mat.additionalValues.find( "metallicRoughnessTexture" );
	//		if ( p_keyValue != valuesEnd )         {
	//			materialData.metallicRoughnessTexture = out.textures[p_keyValue->second.TextureIndex( ) + dummyTextures.Num()].handle;
	//		}

	//		p_keyValue = mat.additionalValues.find( "alphaMode" );
	//		if ( p_keyValue != valuesEnd )         {
	//			if ( p_keyValue->second.string_value == "BLEND" )             {
	//				transparency_mode = TransparencyMode::BLENDED;
	//			}             else if ( p_keyValue->second.string_value == "MASK" )             {
	//				transparency_mode = TransparencyMode::MASKED;
	//			}
	//		}

	//		p_keyValue = mat.additionalValues.find( "alphaCutoff" );
	//		if ( p_keyValue != valuesEnd )         {
	//			materialData.alphaCutoff = static_cast< float >( p_keyValue->second.Factor( ) );
	//		}

	//		bgfxMaterialRef.TransparencyMode = transparency_mode;
	//	}
	//}

	return &out;
}

gltfAssetExplorer::gltfAssetExplorer( )  
{
	guiVisible = true;
	selectedFileHash = 0;
	selectedImage = nullptr;
	selectedMesh = nullptr;

} 
gltfAssetExplorer::~gltfAssetExplorer( ) 
{
}
void gltfAssetExplorer::Init()
{
	renderTarget.width = r_gltfEditRenderWidth.GetInteger();
	renderTarget.height = r_gltfEditRenderHeight.GetInteger( );
	renderTarget.viewId = 10;

	static auto * thisPtr = this;
	bgfxRegisterCallback([](bgfxContext_t * context ) 
		-> auto {
		thisPtr->imDraw(context);
		thisPtr->Render(context);
	} );

	float fov = 27.f;
	float camYAngle = 165.f / 180.f * 3.14159f;
	float camXAngle = 32.f / 180.f * 3.14159f;
	bx::Vec3 eye = { cosf( camYAngle ) * cosf( camXAngle ) * 10, sinf( camXAngle ) * 10, sinf( camYAngle ) * cosf( camXAngle ) * 10 };
	bx::Vec3 at = { 0.f, 0.f, 0.f };
	bx::Vec3 up = { 0.f, 1.f, 0.f };
	bx::mtxLookAt( cameraView.ToFloatPtr( ), eye, at, up, bx::Handness::Right );
	bx::mtxProj(
		cameraProjection.ToFloatPtr( ), 30.0, float( renderTarget.width ) / float( renderTarget.height ), 0.1f,
		10000.0f, bgfx::getCaps( )->homogeneousDepth, bx::Handness::Right );
	camPos = idVec3(eye.x,eye.y,eye.z);


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
	if (!bgfx::isValid( renderTarget.fbh ))
		bgfxCreateMrtTarget( renderTarget, "AssetExplorerView" );

	bgfx::touch( renderTarget.viewId );
	static bgfx::UniformHandle  g_AttribLocationTex = bgfx::createUniform( "colorUniformHandle", bgfx::UniformType::Sampler );
	if (selectedMesh != nullptr )
	{
		bgfx::touch( renderTarget.viewId );
		bgfx::setViewTransform( renderTarget.viewId, cameraView.ToFloatPtr( ), cameraProjection.ToFloatPtr( ) );

		float modelTransform[16];
		bx::mtxIdentity( modelTransform );
		//bx::mtxMul( tmp, modelScale, xtmp );
		//bx::mtxMul( modelTransform, tmp, modelTranslation );
	    bgfx::setTransform( modelTransform );
		bgfx::setUniform(context->pbrContext.u_normalTransform,&modelTransform);
		auto & matList = currentData->MaterialList();
		auto & texList = currentData->TextureList();
		auto & imgList = currentData->ImageList();
		auto & smpList = currentData->SamplerList();
		for ( auto prim : selectedMesh->primitives )
		{

			if ( prim->material != -1 ) 
			{
				gltfMaterial *material = matList[prim->material];
				//prim->material 
				if ( material->pbrMetallicRoughness.baseColorTexture.index != -1 ) 			{
					gltfTexture *texture = texList[material->pbrMetallicRoughness.baseColorTexture.index];
					gltfSampler *sampler = smpList[texture->sampler];
					gltfImage *image = imgList[texture->source];
					bgfx::setTexture( 0, g_AttribLocationTex, image->bgfxTexture.handle );
				}
			}
			//if ( selectedImage )
			//	bgfx::setTexture( 0, g_AttribLocationTex, selectedImage->bgfxTexture.handle );

			bgfx::setVertexBuffer( 0, prim->vertexBufferHandle );
			bgfx::setIndexBuffer( prim->indexBufferHandle );
			bgfx::submit( renderTarget.viewId, context->pbrContext.pbrProgram );
		}

		if ( bgfx::isValid( renderTarget.rb ) )
			bgfx::blit( 100 - renderTarget.viewId, renderTarget.rb, 0, 0, renderTarget.fbTextureHandles[0] );
	    
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
		selectedFileHash = gltfData::fileDataHash.GenerateKey( fileList.begin( ).p->c_str( ) );

	auto curCtx = ImGui::GetCurrentContext();
	ImGui::SetNextWindowPos( idVec2(0,0), ImGuiCond_FirstUseEver );

	ImGuiIO &io = ImGui::GetIO( );

	static idVec2 localMousePos;
	static const idVec2 viewGizmoSize = idVec2( 125.f, 125.f );
	static idVec2 localWindowPos, localWindowSize;

	idVec2 viewGizmoPos;

	static bool gizmoHover = false;
	static bool scrolling = false;
	static int localScrolY = 0;
	ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
	if ( gizmoHover )
		flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

	if (ImGui::Begin("Asset Explorer",&guiVisible, flags ))
	{
		localWindowPos = ImGui::GetWindowPos( );
		localWindowSize = ImGui::GetWindowSize( );
		localMousePos = idVec2( io.MousePos.x - ImGui::GetWindowPos( ).x, io.MousePos.y - ImGui::GetWindowPos( ).y );
		viewGizmoPos = ImVec2( localWindowPos.x + localWindowSize.x - ( viewGizmoSize.x + 20 ), localWindowPos.y + 20 );

		if ( !scrolling && io.MouseDown[0] )
			scrolling = localScrolY != ImGui::GetScrollY( );
		else if ( io.MouseReleased[0] )
			scrolling = false;

		localScrolY = ImGui::GetScrollY( );

		gizmoHover = localMousePos.x > ( localWindowSize.x - viewGizmoSize.x - 50 );
		gizmoHover &= localMousePos.y < ( viewGizmoSize.y + 50 );
		gizmoHover |= ImGui::IsItemActive( );

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
			int currentFileHash = gltfData::fileDataHash.GenerateKey(file);
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

		if ( selectedMesh != nullptr )
		{
			if (io.MouseDown[1] )
			{
				if ( ImGui::IsKeyDown( SDL_SCANCODE_W ) )
				{
					camPos += camAngle.ToForward() * com_frameTime;
				}
				if ( ImGui::IsKeyDown( SDL_SCANCODE_W ) )
				{
					camPos -= camAngle.ToForward( ) * com_frameTime;
				}
			}
			ImGui::Image((void*)(intptr_t)renderTarget.rb.idx, idVec2( ( float ) renderTarget.width / 2, ( float ) renderTarget.height / 2 ), idVec2( 0.0f, 0.0f ), idVec2( 1.0f, 1.0f ) );

			float windowWidth = ( float ) ImGui::GetWindowWidth( );
			float windowHeight = ( float ) ImGui::GetWindowHeight( );
			ImGuizmo::SetDrawlist();
			ImGuizmo::AllowAxisFlip( false );
			ImGuizmo::SetOrthographic( false );
			ImGuizmo::SetRect( ImGui::GetWindowPos( ).x, ImGui::GetWindowPos( ).y, windowWidth, windowHeight );
			if ( !scrolling && ImGui::IsWindowFocused( ) )
				ImGuizmo::ViewManipulate( cameraView.ToFloatPtr( ), 10, viewGizmoPos, viewGizmoSize, 0x10101010 );

			
		}
		else if ( selectedImage != nullptr )
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
					imageCount++;
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
							currentData = data;
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
