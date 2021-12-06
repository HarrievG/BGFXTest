#include "gltfEditor.h"
#include "imgui.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

gltfSceneEditor::gltfSceneEditor( )
	: mWindowOpen(false)
{}

void gltfSceneEditor::Render( ) {
	
}

void gltfSceneEditor::DrawUI( ) {
	auto curCtx = ImGui::GetCurrentContext();
	ImGui::SetNextWindowPos( ImVec2(0,0), ImGuiCond_Once );
	if (ImGui::Begin("GLTF SCENE",&mWindowOpen, ImGuiWindowFlags_MenuBar))
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
	}
	ImGui::End();
}

bool gltfSceneEditor::LoadFile( const char *binFile ) 
{
	std::string ext = "";//tinygltf::GetFilePathExtension( binFile );
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;
	bool ret = false;
	if ( ext.compare( "glb" ) == 0 ) {
		// assume binary glTF.
		ret =
			loader.LoadBinaryFromFile( &model, &err, &warn, binFile );
	} else {
		// assume ascii glTF.
		ret = loader.LoadASCIIFromFile( &model, &err, &warn, binFile );
	}

	if ( !warn.empty( ) ) {
		printf( "Warn: %s\n", warn.c_str( ) );
	}

	if ( !err.empty( ) ) {
		printf( "ERR: %s\n", err.c_str( ) );
	}
	if ( !ret ) {
		printf( "Failed to load .glTF : %s\n", binFile );
		exit( -1 );
	}

	return true;
}
