#include "stdafx.h"
#include "UIEditor.h"

#include "Timer.h"
#include "Graphics.h"
#include "FileLoading.h"
#include <algorithm>

#if _DEBUG
#include <imgui/imgui.h>
#endif

#define FOLDER_PATH "Resources\\UI\\"
#define FOLDER_PATH_SCREENS "Resources\\UI\\Screens\\"

UIEditor::UIEditor()
{
	LoadFromFile_Screens();
	LoadFromFile_Widgets();
}

UIEditor::~UIEditor() { }

void UIEditor::LoadFromFile_Screens()
{
	// Load UI screens
	JsonLoading::LoadJson( m_vUIScreenList, FOLDER_PATH + m_sJsonFile );
	SortScreens();
}

void UIEditor::LoadFromFile_Widgets()
{
	// Load screen widgets
	for ( unsigned int i = 0; i < m_vUIScreenList.size(); i++ )
	{
		std::vector<UIScreenData> screenData;
		JsonLoading::LoadJson( screenData, FOLDER_PATH_SCREENS + m_vUIScreenList[i].file );
		m_vUIScreenData.emplace( m_vUIScreenList[i].name, screenData );
	}
}

void UIEditor::SortScreens()
{
	// Sort screens by name for ImGui
	std::vector<std::string> screenNames;
	for ( unsigned int i = 0; i < m_vUIScreenList.size(); i++ )
		screenNames.push_back( m_vUIScreenList[i].name );
	sort( screenNames.begin(), screenNames.end() );
	std::vector<UIScreenList> tempScreenList;
	for ( unsigned int i = 0; i < screenNames.size(); i++ )
	{
		for ( unsigned int j = 0; j < m_vUIScreenList.size(); j++ )
		{
			if ( screenNames[i] == m_vUIScreenList[j].name )
			{
				tempScreenList.push_back( m_vUIScreenList[j] );
			}
		}
	}
	m_vUIScreenList = tempScreenList;
}

#if _DEBUG
void UIEditor::SaveToFile_Screens()
{
	JsonLoading::SaveJson( m_vUIScreenList, FOLDER_PATH + m_sJsonFile );
}

void UIEditor::SaveToFile_Widgets()
{
	for ( unsigned int i = 0; i < m_vUIScreenList.size(); i++ )
	{
		for ( std::map<std::string, std::vector<UIScreenData>>::iterator it = m_vUIScreenData.begin(); it != m_vUIScreenData.end(); it++ )
		{
			if ( m_vUIScreenList[i].name == it->first )
			{
				JsonLoading::SaveJson( it->second, FOLDER_PATH_SCREENS + m_vUIScreenList[i].file );
			}
		}
	}
}

void UIEditor::SpawnControlWindow( const Graphics& gfx )
{
	static Timer timer;
	static float counter = 0.0f;
	static bool savedFile = false;
	static int currentScreenIdx = -1;

	if ( ImGui::Begin( "UI Editor", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		// Save UI Screens
		if ( ImGui::Button( "Save screens?" ) )
		{
			SaveToFile_Screens();
			savedFile = true;
		}
		ImGui::SameLine();

		// Save UI components
		if ( ImGui::Button( "Save widgets?" ) )
		{
			SaveToFile_Widgets();
			savedFile = true;
		}

		// Update save message
		if ( savedFile )
		{
			ImGui::TextColored( ImVec4( 0.1f, 1.0f, 0.1f, 1.0f ), "FILE SAVED!" );
			counter += timer.GetDeltaTime();
			if ( counter > 3.0f )
			{
				counter = 0.0f;
				savedFile = false;
			}
		}
		ImGui::NewLine();

		if ( ImGui::CollapsingHeader( "Screens", ImGuiTreeNodeFlags_DefaultOpen ) )
		{
			// List of all UI screens currently defined
			ImGui::Text( "UI Screen List" );
			if ( ImGui::BeginListBox( "##UI Screen List", ImVec2( -FLT_MIN, m_vUIScreenList.size() * ImGui::GetTextLineHeightWithSpacing() * 1.1f ) ) )
			{
				int index = 0;
				for ( auto const& [key, value] : m_vUIScreenList )
				{
					const bool isSelected = ( currentScreenIdx == index );
					if ( ImGui::Selectable( key.c_str(), isSelected ) )
						currentScreenIdx = index;

					if ( isSelected )
						ImGui::SetItemDefaultFocus();
				
					index++;
				}
				ImGui::EndListBox();
			}

			// Modify screens
			if ( currentScreenIdx > -1 )
			{
				// Update screen name
				static char buf[32] = "";
				static bool modifiedName = false;
				ImGui::Text( "Screen Name: " );
				ImGui::SameLine();
				ImGui::TextColored( ImVec4( 1.0f, 0.1f, 0.1f, 1.0f ), m_vUIScreenList[currentScreenIdx].name.c_str() );
				if ( ImGui::InputText( std::string( "##" ).append( m_vUIScreenList[currentScreenIdx].name ).c_str(), buf, IM_ARRAYSIZE( buf ) ) )
					modifiedName = true;
				if ( modifiedName )
				{
					if ( ImGui::Button( std::string( "Save Name##" ).append( m_vUIScreenList[currentScreenIdx].name ).c_str() ) )
					{
						// TODO: prevent user from setting duplicate names
						m_vUIScreenList[currentScreenIdx].name = buf;
						modifiedName = false;
					}
				}
				ImGui::NewLine();

				// Update screen widget file
				ImGui::Text( "Screen Widget File: " );
				ImGui::SameLine();
				ImGui::TextColored( ImVec4( 1.0f, 0.1f, 0.1f, 1.0f ), m_vUIScreenList[currentScreenIdx].file.c_str() );
				if ( ImGui::Button( "Load Widget File" ) )
				{
					if ( FileLoading::OpenFileExplorer( m_sSelectedFile, m_sFilePath ) )
					{
						m_vUIScreenList[currentScreenIdx].file = m_sSelectedFile;
						std::string type = ".json";
						std::string::size_type idx = m_sSelectedFile.find( type );
						if ( idx != std::string::npos )
							m_sSelectedFile.erase( idx, type.length() );
						m_vUIScreenList[currentScreenIdx].name = m_sSelectedFile;

						SortScreens();
						LoadFromFile_Widgets();
					}
				}
				ImGui::NewLine();

				// Add/remove screens
				if ( ImGui::Button( "Add New Screen" ) )
				{
					static int screenIdx = 0;
					std::string screenName = "Blank Screen " + std::to_string( screenIdx );
					m_vUIScreenList.push_back( UIScreenList( { screenName, std::string( screenName ).append( ".json" ) } ) );
					currentScreenIdx -= m_vUIScreenList.size() - 1;
				}
				ImGui::SameLine();
				if ( ImGui::Button( "Remove Current Screen" ) )
				{
					if ( m_vUIScreenList.size() > 1 )
					{
						m_vUIScreenList.erase( m_vUIScreenList.begin() + currentScreenIdx );
						m_vUIScreenList.shrink_to_fit();
						currentScreenIdx -= 1;
					}
				}
			}
		}
		ImGui::NewLine();
		
		if ( ImGui::CollapsingHeader( "Widgets", ImGuiTreeNodeFlags_DefaultOpen ) )
		{

			// Edit UI components for each screen
			int index = 0;
			for ( auto& [key, value] : m_vUIScreenData ) // loop each screens data struct
			{
				if ( index == currentScreenIdx )
				{
					for ( unsigned int i = 0; i < value.size(); i++ ) // loop ui elements on current screen
					{
						if ( ImGui::TreeNode( std::string( value[i].name ).append( " (" + value[i].type + ")" ).c_str() ) )
						{
							ImGui::NewLine();

							// TODO: prevent user from setting duplicate names
							const int bufSize = 32;
							static char buf[bufSize] = "";
							static bool modifiedName = false;
							ImGui::Text( "Widget Name: " );
							ImGui::SameLine();
							ImGui::TextColored( ImVec4( 1.0f, 0.1f, 0.1f, 1.0f ), value[i].name.c_str() );
							if ( ImGui::InputText( std::string( "##" ).append( value[i].name ).c_str(), buf, IM_ARRAYSIZE( buf ) ) )
								modifiedName = true;
							if ( modifiedName )
							{
								if ( ImGui::Button( std::string( "Save Name##" ).append( value[i].name ).c_str() ) )
								{
									value[i].name = buf;
									memset( buf, 0, bufSize );
									modifiedName = false;
								}
							}
							ImGui::NewLine();

							ImGui::Text( "Widget Type: " );
							ImGui::SameLine();
							ImGui::TextColored( ImVec4( 1.0f, 0.1f, 0.1f, 1.0f ), value[i].type.c_str() );
							static int currentType = 0;
							const char* comboPreview = value[i].type.c_str();
							if ( ImGui::BeginCombo( std::string( "##" ).append( value[i].name ).append( value[i].type ).c_str(), comboPreview ) )
							{
								for ( unsigned int j = 0; j < m_vUITypes.size(); j++ )
								{
									const bool isSelected = ( currentType == j );
									if ( ImGui::Selectable( m_vUITypes[j].c_str(), isSelected ) )
									{
										currentType = j;
										value[i].type = m_vUITypes[j];
									}

									if ( isSelected )
										ImGui::SetItemDefaultFocus();
								}
								ImGui::EndCombo();
							}
							// TODO: the widget should change depending on what the user sets
							ImGui::NewLine();

							ImGui::Text( "Position" );
							float max = ( gfx.GetWidth() > gfx.GetHeight() ? gfx.GetWidth() : gfx.GetHeight() );
							static float position[2] = { value[i].position[0], value[i].position[1] };
							ImGui::SliderFloat2( std::string( "##Position" ).append( key ).append( value[i].name ).c_str(), position, 0.0f, max, "%.1f" );
							value[i].position = { position[0], position[1] };
							ImGui::NewLine();

							ImGui::Text( "Scale" );
							static float scale[2] = { value[i].scale[0], value[i].scale[1] };
							ImGui::SliderFloat2( std::string( "##Scale" ).append( key ).append( value[i].name ).c_str(), scale, 0.0f, max, "%.1f" );
							value[i].scale = { scale[0], scale[1] };
							ImGui::NewLine();

							// Remove the current widget?
							if ( ImGui::Button( "Remove Widget" ) )
							{
								value.erase( value.begin() + i );
								value.shrink_to_fit();
							}
							ImGui::NewLine();

							ImGui::TreePop();
						}
					}
				}
				index++;
			}

			// Add a new widget
			if ( ImGui::Button( "Add New Widget" ) )
			{
				std::string screenName = m_vUIScreenList[currentScreenIdx].name;
				for ( auto& [key, value] : m_vUIScreenData )
				{
					if ( key == screenName )
					{
						static int widgetIdx = 0;
						value.push_back( UIScreenData( "Blank Widget " + std::to_string( widgetIdx ), "Image", { 0.0f, 0.0f }, { 64.0f, 64.0f } ) );
					}
				}
			}
		}
	}
	ImGui::End();
}
#endif