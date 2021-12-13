#include "gltfEditor.h"
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// no need for it, it should go,  but doenst compile ootb..
//#define TINYGLTF_NO_FS
#include "tiny_gltf.h"
#include "imgui.h"

#include "CVarSystem.h"
#include "Common.h"
#include "FileSystem.h"
#include "CVarSystem.h"


tinygltf::TinyGLTF gGLFTLoader;

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
	static auto *thisPtr = this;
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
		for (auto &asset : thisPtr->GetLoadedAssets())
		{
			for (auto& key : asset.asset.extras.Keys() )
				common->Printf( "  %-21s : %-21s\n", key.c_str(),
					asset.asset.extras.Get(key).Get<std::string>().c_str() );
		}
	}, CMD_FL_RENDERER, "lists all loaded gltf models" );
}
void gltfSceneEditor::Render( ) {

	//float model[16];
	//bx::mtxIdentity( model );
	//bgfx::setTransform( model );

	//bgfx::setVertexBuffer( 0, context->vbh );
	//bgfx::setIndexBuffer( context->ibh );

	//bgfx::submit( 0, context->program );
}

void gltfSceneEditor::DrawUI( ) {
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
				if ( ImGui::MenuItem( "Function Editor" ) ) {
				}//m_FuncEditor->ShowWindow( true );

				if ( ImGui::MenuItem( "Material Editor" ) ) {
				}//m_MaterialEditor->ShowWindow( true );

				ImGui::EndMenu( );
			}
			ImGui::EndMenuBar( );
		}

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
	}
	ImGui::End();
}
bool gltfSceneEditor::IsFileLoaded(const char *file )
{
	return loadedFiles.FindIndex( idStr(file) ) != -1;
}

bool gltfSceneEditor::LoadFile( const char *binFile ) 
{
	if ( IsFileLoaded(binFile) )
	{
		if ( cvarSystem->GetCVarInteger( "fs_debug" ))
			common->DPrintf( "%s was already loaded \n", binFile );
		return true;
	}

	std::string ext = tinygltf::GetFilePathExtension( binFile );
	tinygltf::Model &model = loadedAssets.Alloc();
	std::string err;
	std::string warn;
	bool ret = false;
	if ( ext.compare( "glb" ) == 0 || ext.compare( "bin" ) == 0 ) {
		common->DPrintf( "Loading %s as a binary\n", binFile );
		// assume binary glTF.
		ret = gGLFTLoader.LoadBinaryFromFile( &model, &err, &warn, binFile );
	} else {
		// assume ascii glTF.
		common->DPrintf( "Loading %s as a ASCII\n", binFile );
		ret = gGLFTLoader.LoadASCIIFromFile( &model, &err, &warn, binFile );
	}

	if ( !warn.empty( ) ) {
		common->Warning( "%s", warn.c_str( ) );
	}

	if ( !err.empty( ) ) {
		common->Error( "%s", err.c_str( ) );
	}
	if ( !ret ) {
		common->Error( "Failed to load .glTF : %s\n", binFile );
	}
	loadedFiles.Append(	binFile );
	common->Printf( "^2Loading %s Done",binFile );
	return true;
}
