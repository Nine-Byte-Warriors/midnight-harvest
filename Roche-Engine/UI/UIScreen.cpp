#include "stdafx.h"
#include "UIScreen.h"
#include "Graphics.h"
#include "AudioEngine.h"
#include <shellapi.h>

#if _DEBUG
extern bool g_bDebug;
#endif
#define RENDER_IF_IN_BOX( x, y, z, code ) if ( x >= y && x <= ( y + z ) ) code

void UIScreen::Initialize( const Graphics& gfx, ConstantBuffer<Matrices>* mat, const std::vector<std::shared_ptr<Widget>>& widgets, Health& health, Inventory& inventory )
{
	m_cbMatrices = mat;
	m_vWidgets = widgets;
	m_pInventory = &inventory;
	m_pPlayerHealth = &health;
	m_pDevice = gfx.GetDevice();
	m_pContext = gfx.GetContext();
	InitializeWidgets();
}

void UIScreen::InitializeWidgets()
{
	for ( unsigned int i = 0; i < m_vWidgets.size(); i++ )
	{
		m_vWidgets[i]->GetButtonWidget()->Initialize( m_pDevice.Get(), m_pContext.Get(), *m_cbMatrices );
		m_vWidgets[i]->GetColourBlockWidget()->Initialize( m_pDevice.Get(), m_pContext.Get(), *m_cbMatrices );
		m_vWidgets[i]->GetDataSliderWidget()->Initialize( m_pDevice.Get(), m_pContext.Get(), *m_cbMatrices, i );
		m_vWidgets[i]->GetDropDownWidget()->Initialize( m_pDevice.Get(), m_pContext.Get(), *m_cbMatrices );
		m_vWidgets[i]->GetEnergyBarWidget()->Initialize( m_pDevice.Get(), m_pContext.Get(), *m_cbMatrices );
		m_vWidgets[i]->GetImageWidget()->Initialize( m_pDevice.Get(), m_pContext.Get(), *m_cbMatrices );
		m_vWidgets[i]->GetInputWidget()->Initialize( m_pDevice.Get(), m_pContext.Get(), *m_cbMatrices, i );
		m_vWidgets[i]->GetPageSliderWidget()->Initialize( m_pDevice.Get(), m_pContext.Get(), *m_cbMatrices, m_vScreenSize.x, m_vScreenSize.y );
	}
}

void UIScreen::Update( const float dt )
{
	if ( m_mouseData.Hover )
		m_mouseData.Hover = false;

	if ( !m_mouseData.LPress )
		m_mouseData.Locked = false;

	// first, determine if any of the widgets is a page slider
	m_bUsingPageSlider = false;
	for ( unsigned int i = 0; i < m_vWidgets.size(); i++ )
	{
		if ( m_vWidgets[i]->GetType() == "Page Slider" )
		{
			m_bUsingPageSlider = true;
			break;
		}
	}

	for ( unsigned int i = 0; i < m_vWidgets.size(); i++ )
	{
		if ( m_bUsingPageSlider && m_vWidgets[i]->GetType() != "Page Slider" )
			m_vWidgets[i]->SetPosition( { m_vWidgets[i]->GetPosition().x, m_vWidgets[i]->GetPosition().y + ( ( m_fCurrentY / m_vScreenSize.y ) * 100.0f ) } );

#pragma region BUTTONS
		if ( m_vWidgets[i]->GetType() == "Button" )
		{
			static std::vector<std::string> SeedStrings = { "Bean", "Carrot", "Cauliflower", "Onion", "Potato", "Tomato" };
			for ( unsigned int j = 0; j < SeedStrings.size(); j++ )
			{
				if ( m_vWidgets[i]->GetAction() == ( SeedStrings[j] + " Background" ) )
				{
					m_vWidgets[i]->GetButtonWidget()->SetTextOffset( XMFLOAT2( 40.0f, 30.0f ) );
					if ( !m_vWidgets[i]->GetIsHidden() )
					{
						if ( m_vWidgets[i]->GetButtonWidget()->Resolve(
							std::to_string( m_pInventory->GetSeedPacketCount( SeedStrings[j] ) ),
							Colors::White, m_textures, m_mouseData, m_pInventory->IsActiveSeedPacket( j ) ) )
						{
							m_pInventory->SetActiveSeedPacket( j );
						}
					}
				}
			}

			if ( m_vWidgets[i]->GetAction() == "Link" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "", Colors::White, m_texturesGithub, m_mouseData ) )
						if ( !m_bOpenLink && m_bOpen )
							m_bOpenLink = true;

				if ( !m_vWidgets[i]->GetButtonWidget()->GetIsPressed() )
					m_bOpen = true;

				if ( m_bOpenLink )
				{
					ShellExecute( 0, 0, L"https://github.com/Nine-Byte-Warriors", 0, 0, SW_SHOW );
					m_bOpenLink = false;
					m_bOpen = false;
				}
			}
			if (m_vWidgets[i]->GetAction() == "End Phase")
			{
				if (!m_vWidgets[i]->GetIsHidden())
					if (m_vWidgets[i]->GetButtonWidget()->Resolve("End Phase", Colors::White, m_textures, m_mouseData, false, FontSize::LARGE))
						EventSystem::Instance()->AddEvent(EVENTID::ChangePhase);
			}
			if ( m_vWidgets[i]->GetAction() == "Close" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "Quit Game", Colors::White, m_textures, m_mouseData, false, FontSize::LARGE ) )
						EventSystem::Instance()->AddEvent( EVENTID::QuitGameEvent );
			}
			if ( m_vWidgets[i]->GetAction() == "Back To Menu" )
			{
				if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "Back To Menu", Colors::White, m_textures, m_mouseData, false, FontSize::LARGE ) )
					EventSystem::Instance()->AddEvent( EVENTID::FadeToBlack_Game );
			}
			if ( m_vWidgets[i]->GetAction() == "Start" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "Start Game", Colors::White, m_textures, m_mouseData, false, FontSize::LARGE ) )
						EventSystem::Instance()->AddEvent( EVENTID::FadeToBlack_Start );
			}
			if ( m_vWidgets[i]->GetAction() == "Settings" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "Settings", Colors::White, m_textures, m_mouseData, false, FontSize::LARGE) )
						EventSystem::Instance()->AddEvent( EVENTID::OpenSettings );
			}
			if ( m_vWidgets[i]->GetAction() == "Credits" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "Credits", Colors::White, m_textures, m_mouseData, false, FontSize::LARGE) )
						EventSystem::Instance()->AddEvent( EVENTID::OpenCredits );
			}
			if (m_vWidgets[i]->GetAction() == "Leaderboard")
			{
				if (m_vWidgets[i]->GetButtonWidget()->Resolve("Scoreboard", Colors::White, m_textures, m_mouseData, false, FontSize::LARGE))
					EventSystem::Instance()->AddEvent(EVENTID::OpenLeaderboard);
			}
			if ( m_vWidgets[i]->GetAction() == "Resume" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "Resume", Colors::White, m_textures, m_mouseData, false, FontSize::LARGE) )
						EventSystem::Instance()->AddEvent( EVENTID::ResumeGame );
			}
			if ( m_vWidgets[i]->GetAction() == "Back" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "Back", Colors::White, m_textures, m_mouseData, false, FontSize::LARGE) )
						EventSystem::Instance()->AddEvent( EVENTID::Back );
			}
			if (m_vWidgets[i]->GetAction() == "Restart")
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if (m_vWidgets[i]->GetButtonWidget()->Resolve("Restart", Colors::White, m_textures, m_mouseData, false ,FontSize::LARGE))
						EventSystem::Instance()->AddEvent(EVENTID::FadeToBlack_GameRestart);
			}
			if (m_vWidgets[i]->GetAction() == "Confirm")
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if (m_vWidgets[i]->GetButtonWidget()->Resolve("Yes", Colors::White, m_textures, m_mouseData, false, FontSize::LARGE))
						EventSystem::Instance()->AddEvent(EVENTID::FadeToBlack_Shop);
			}
			if (m_vWidgets[i]->GetAction() == "Deny")
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if (m_vWidgets[i]->GetButtonWidget()->Resolve("No", Colors::White, m_textures, m_mouseData, false, FontSize::LARGE))
						EventSystem::Instance()->AddEvent(EVENTID::CloseUIPopUp);
			}

			if ( m_vWidgets[i]->GetAction() == "General" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "", Colors::White, m_texturesGeneralTabs, m_mouseData, m_eTabsState == Tabs::General ) )
						m_eTabsState = Tabs::General;
			}
			if ( m_vWidgets[i]->GetAction() == "Graphics" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "", Colors::White, m_texturesGraphicsTabs, m_mouseData, m_eTabsState == Tabs::Graphics ) )
						m_eTabsState = Tabs::Graphics;
			}
			if ( m_vWidgets[i]->GetAction() == "Music" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "", Colors::White, m_texturesMusicTabs, m_mouseData, m_eTabsState == Tabs::Audio ) )
						m_eTabsState = Tabs::Audio;
			}
			if ( m_vWidgets[i]->GetAction() == "Control" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					if ( m_vWidgets[i]->GetButtonWidget()->Resolve( "", Colors::White, m_texturesControlTabs, m_mouseData, m_eTabsState == Tabs::Controls ) )
						m_eTabsState = Tabs::Controls;
			}
			if ( m_vWidgets[i]->GetAction() == "" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					m_vWidgets[i]->GetButtonWidget()->Resolve( "", Colors::White, m_textures, m_mouseData );
			}
			m_vWidgets[i]->GetButtonWidget()->Update( dt );
		}
#pragma endregion

#pragma region COLOUR_BLOCKS
		if ( m_vWidgets[i]->GetType() == "Colour Block" )
		{
			// Doesn't need actions
			m_vWidgets[i]->GetColourBlockWidget()->Resolve( { 210, 210, 150 } );
			m_vWidgets[i]->GetColourBlockWidget()->Update( dt );
		}
#pragma endregion

#pragma region DATA_SLIDERS
		if ( m_vWidgets[i]->GetType() == "Data Slider" )
		{
			if ( m_vWidgets[i]->GetAction() == "Master Volume" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Audio ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetDataSliderWidget()->Resolve(
						"Resources\\Textures\\UI\\Slider\\Slider Background.png",
						"Resources\\Textures\\UI\\Slider\\Control Point.png", m_mouseData, i, AudioEngine::GetInstance()->GetMasterVolume() * 100);
					float sliderData = (float)m_vWidgets[i]->GetDataSliderWidget()->GetData() / 100.0f;
					AudioEngine::GetInstance()->SetMasterVolume( sliderData );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "Music Volume" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Audio ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetDataSliderWidget()->Resolve(
						"Resources\\Textures\\UI\\Slider\\Slider Background.png",
						"Resources\\Textures\\UI\\Slider\\Control Point.png", m_mouseData, i, AudioEngine::GetInstance()->GetMusicVolume() * 100);
					float sliderData = (float)m_vWidgets[i]->GetDataSliderWidget()->GetData() / 100.0f;
					AudioEngine::GetInstance()->SetMusicVolume( sliderData );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "SFX Volume" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Audio ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetDataSliderWidget()->Resolve(
						"Resources\\Textures\\UI\\Slider\\Slider Background.png",
						"Resources\\Textures\\UI\\Slider\\Control Point.png", m_mouseData, i, AudioEngine::GetInstance()->GetSFXVolume() * 100);
					float sliderData = (float)m_vWidgets[i]->GetDataSliderWidget()->GetData() / 100.0f;
					AudioEngine::GetInstance()->SetSFXVolume( sliderData );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "Screen Shake" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::General ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetDataSliderWidget()->Resolve(
						"Resources\\Textures\\UI\\Slider\\Slider Background.png",
						"Resources\\Textures\\UI\\Slider\\Control Point.png", m_mouseData, i );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "Red Overlay Slider" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Graphics ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetDataSliderWidget()->Resolve(
						"Resources\\Textures\\UI\\Slider\\Slider Background.png",
						"Resources\\Textures\\UI\\Slider\\Control Point.png", m_mouseData, i, 100 );
					float* redOverlay = new float( m_vWidgets[i]->GetDataSliderWidget()->GetData() / 100.0f );
					EventSystem::Instance()->AddEvent( EVENTID::RedOverlayColour, redOverlay );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "Green Overlay Slider" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Graphics ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetDataSliderWidget()->Resolve(
						"Resources\\Textures\\UI\\Slider\\Slider Background.png",
						"Resources\\Textures\\UI\\Slider\\Control Point.png", m_mouseData, i, 100 );
					float* greenOverlay = new float( m_vWidgets[i]->GetDataSliderWidget()->GetData() / 100.0f );
					EventSystem::Instance()->AddEvent( EVENTID::GreenOverlayColour, greenOverlay );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "Blue Overlay Slider" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Graphics ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetDataSliderWidget()->Resolve(
						"Resources\\Textures\\UI\\Slider\\Slider Background.png",
						"Resources\\Textures\\UI\\Slider\\Control Point.png", m_mouseData, i, 100 );
					float* blueOverlay = new float( m_vWidgets[i]->GetDataSliderWidget()->GetData() / 100.0f );
					EventSystem::Instance()->AddEvent( EVENTID::BlueOverlayColour, blueOverlay );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					m_vWidgets[i]->GetDataSliderWidget()->Resolve(
						"Resources\\Textures\\UI\\Slider\\Slider Background.png",
						"Resources\\Textures\\UI\\Slider\\Control Point.png", m_mouseData, i );
			}
			m_vWidgets[i]->GetDataSliderWidget()->Update( dt );
		}
#pragma endregion

#pragma region DROP_DOWNS
		if ( m_vWidgets[i]->GetType() == "Drop Down" )
		{
			if ( m_vWidgets[i]->GetAction() == "Resolution" )
			{
				// Create a drop down that allows user to change resolution
			}
			if ( m_vWidgets[i]->GetAction() == "Language" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::General ? false : true );

				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					// Create a drop down that allows user to change Language
					std::vector<std::string> vValues = { "English (UK)", "English (USA)" };
					static std::string sValue = vValues[0];
					m_vWidgets[i]->GetDropDownWidget()->Resolve( vValues, m_texturesDD, m_texturesDDButton, Colors::White, sValue, m_mouseData );
					if ( m_vWidgets[i]->GetDropDownWidget()->GetSelected() == "English (USA)" )
						sValue = "English (USA)";
					else
						sValue = "English (UK)";
				}
			}
			if ( m_vWidgets[i]->GetAction() == "" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					std::vector<std::string> vValues = { "True", "False" };
					static std::string sValue = vValues[0];
					m_vWidgets[i]->GetDropDownWidget()->Resolve( vValues, m_texturesDD, m_texturesDDButton, Colors::White, sValue, m_mouseData );
					if ( m_vWidgets[i]->GetDropDownWidget()->GetSelected() == "False" )
						sValue = "False";
					else
						sValue = "True";
				}
			}
			m_vWidgets[i]->GetDropDownWidget()->Update( dt );
		}
#pragma endregion

#pragma region ENERGY_BARS
		if ( m_vWidgets[i]->GetType() == "Energy Bar" )
		{
			if ( m_vWidgets[i]->GetAction() == "Player Health" )
			{
				// Bar that displays the player's health
			}
			if ( m_vWidgets[i]->GetAction() == "Enemy Health" )
			{
				// Bar that displays an enemy's health
			}
			if ( m_vWidgets[i]->GetAction() == "" )
			{
				if ((*m_fCurrentHealthPtr / *m_fMaxHealthPtr) < 1 && (*m_fCurrentHealthPtr / *m_fMaxHealthPtr) > 0)
				{
					m_fMaxHealth = *m_fMaxHealthPtr;
					m_fCurrentHealth = *m_fCurrentHealthPtr;
				}

				m_fPercentageHealth = 100 * m_fCurrentHealth / m_fMaxHealth;
				if (m_fPercentageHealth > 100)
				{
					m_fPercentageHealth = 100;
				}
				std::string temp = m_textures[2];
				m_textures[2] = "";
				m_vWidgets[i]->GetEnergyBarWidget()->Resolve( m_textures, m_fPercentageHealth);
				m_textures[2] = temp;
			}
			m_vWidgets[i]->GetEnergyBarWidget()->Update( dt );
		}
#pragma endregion

#pragma region IMAGES
		if ( m_vWidgets[i]->GetType() == "Image" )
		{
			if ( m_vWidgets[i]->GetAction() == "Master volume label" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Audio ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Master Volumne", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}
			if ( m_vWidgets[i]->GetAction() == "Music volume label" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Audio ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Music Volumne", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}
			if ( m_vWidgets[i]->GetAction() == "SFX volume label" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Audio ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "SFX Volumne", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}

			if ( m_vWidgets[i]->GetAction() == "Coins" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(std::to_string( m_pInventory->GetCoinCount() ), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}
			if ( m_vWidgets[i]->GetAction() == "Score Label" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "Score", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png" );
			}
			if (m_vWidgets[i]->GetAction() == "Score")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetScoreStr(), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}
			if (m_vWidgets[i]->GetAction() == "Enemy Health Label")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve("Enemies' Health", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}

			if (m_vWidgets[i]->GetAction() == "Restart Pop Up Label")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve("GAME OVER", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::HUGE);
			}
			if (m_vWidgets[i]->GetAction() == "Win Pop Up Label")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve("GAME WON", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::HUGE);
			}
			if (m_vWidgets[i]->GetAction() == "Score Pop Up Label")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetScoreStr(), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::HUGE);
			}

			if (m_vWidgets[i]->GetAction() == "Change Level Label")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve("Change \nLevel?", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE);
			}


			if ( m_vWidgets[i]->GetAction() == "Language DD Label" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::General ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Language", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}
			if ( m_vWidgets[i]->GetAction() == "Screen Shake Slider Label" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::General ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Screen Shake Amount", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}

			if ( m_vWidgets[i]->GetAction() == "Pause Title" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "PAUSED", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::INSANE );
			}
			if ( m_vWidgets[i]->GetAction() == "Up Control" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Up Control ", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}
			if ( m_vWidgets[i]->GetAction() == "Left Control" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Left Control", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}
			if ( m_vWidgets[i]->GetAction() == "Down Control" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Down Control", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}
			if ( m_vWidgets[i]->GetAction() == "Right Control" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Right Control", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}
			if ( m_vWidgets[i]->GetAction() == "Dash Control" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Dash Control", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}
			if ( m_vWidgets[i]->GetAction() == "Interact Control" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Interact Control", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}

			if ( m_vWidgets[i]->GetAction() == "Red Overlay Label" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Graphics ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Red Overlay", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}
			if ( m_vWidgets[i]->GetAction() == "Green Overlay Label" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Graphics ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Green Overlay", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}
			if ( m_vWidgets[i]->GetAction() == "Blue Overlay Label" )
			{
				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Graphics ? false : true );
				m_vWidgets[i]->GetImageWidget()->Resolve( "Blue Overlay", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::VERY_LARGE );
			}

			if ( m_vWidgets[i]->GetAction() == "CreditTitle" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "CREDITS", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::INSANE );
			}
			if ( m_vWidgets[i]->GetAction() == "Credit1" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "Kyle Robinson", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Credit2" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "Juliusz Kaczmarek", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Credit3" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "Rece Thompson-Hamilton", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Credit4" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "Charlie Morris", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Credit5" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "Alvin Aggrey", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Credit6" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "Will Bennett", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Credit7" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "James Barber", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Credit8" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "Said Mozamil Sadat", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Credit9" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "Glen Nelson", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Credit10" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "Eleftherios Karakyritsis", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png" );
			}

			if (m_vWidgets[i]->GetAction() == "LeaderboardTitle")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve("SCOREBOARD", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::INSANE);
			}
			if (m_vWidgets[i]->GetAction() == "ScoreLabel0")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetName(0), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}
			if (m_vWidgets[i]->GetAction() == "ScoreLabel1")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetName(1), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}
			if (m_vWidgets[i]->GetAction() == "ScoreLabel2")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetName(2), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}
			if (m_vWidgets[i]->GetAction() == "ScoreLabel3")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetName(3), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}
			if (m_vWidgets[i]->GetAction() == "ScoreLabel4")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetName(4), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}
			if (m_vWidgets[i]->GetAction() == "Score0")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetScoreStr(0), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}
			if (m_vWidgets[i]->GetAction() == "Score1")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetScoreStr(1), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}
			if (m_vWidgets[i]->GetAction() == "Score2")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetScoreStr(2), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}
			if (m_vWidgets[i]->GetAction() == "Score3")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetScoreStr(3), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}
			if (m_vWidgets[i]->GetAction() == "Score4")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve(m_scoreBoard.GetScoreStr(4), Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png");
			}

			if (m_vWidgets[i]->GetAction() == "Loading Label")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve("Loading ...", Colors::AntiqueWhite, "Resources\\Textures\\Tiles\\transparent.png", FontSize::INSANE);
			}


			std::string currTexture = m_texturesHearts[0];
			float currHealth = m_pPlayerHealth->GetCurrentHealth();
			float maxHealth = m_pPlayerHealth->GetMaxHealth();

			if ( m_vWidgets[i]->GetAction() == "Carrot Seed Packet" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "", Colors::AntiqueWhite, "Resources\\Textures\\UI\\Seeds\\Carrot Seeds.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Cauliflower Seed Packet" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "", Colors::AntiqueWhite, "Resources\\Textures\\UI\\Seeds\\Cauliflower Seeds.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Bean Seed Packet" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "", Colors::AntiqueWhite, "Resources\\Textures\\UI\\Seeds\\Bean Seeds.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Onion Seed Packet" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "", Colors::AntiqueWhite, "Resources\\Textures\\UI\\Seeds\\Onion Seeds.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Potato Seed Packet" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "", Colors::AntiqueWhite, "Resources\\Textures\\UI\\Seeds\\Potato Seeds.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Tomato Seed Packet" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "", Colors::AntiqueWhite, "Resources\\Textures\\UI\\Seeds\\Tomato Seeds.png" );
			}
			if ( m_vWidgets[i]->GetAction() == "Heart_1" )
			{
				switch ( (int)currHealth )
				{
				case 0: currTexture = m_texturesHearts[2]; break;
				case 1: currTexture = m_texturesHearts[1]; break;
				case 2: case 3: case 4: case 5: case 6: currTexture = m_texturesHearts[0]; break;
				}
				m_vWidgets[i]->GetImageWidget()->Resolve( "", Colors::AntiqueWhite, currTexture );
			}
			if ( m_vWidgets[i]->GetAction() == "Heart_2" )
			{
				switch ( (int)currHealth )
				{
				case 0: case 1: case 2: currTexture = m_texturesHearts[2]; break;
				case 3: currTexture = m_texturesHearts[1]; break;
				case 4: case 5: case 6: currTexture = m_texturesHearts[0]; break;
				}
				m_vWidgets[i]->GetImageWidget()->Resolve( "", Colors::AntiqueWhite, currTexture );
			}
			if ( m_vWidgets[i]->GetAction() == "Heart_3" )
			{
				switch ( (int)currHealth )
				{
				case 0: case 1: case 2: case 3: case 4: currTexture = m_texturesHearts[2]; break;
				case 5: currTexture = m_texturesHearts[1]; break;
				case 6: currTexture = m_texturesHearts[0]; break;
				}
				m_vWidgets[i]->GetImageWidget()->Resolve( "", Colors::AntiqueWhite, currTexture );
			}
			if ( m_vWidgets[i]->GetAction() == "Coin Icon" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "", Colors::AntiqueWhite, "Resources\\Textures\\UI\\Coin\\Coin.png" );
			}
			if (m_vWidgets[i]->GetAction() == "Title Card")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve("", Colors::AntiqueWhite, "Resources\\Textures\\UI\\Title\\Title.png");
			}
			if (m_vWidgets[i]->GetAction() == "Backdrop")
			{
				m_vWidgets[i]->GetImageWidget()->Resolve("", Colors::AntiqueWhite, "Resources\\Textures\\UI\\Backdrop\\Main menu backdrop.png");
			}
			if ( m_vWidgets[i]->GetAction() == "" )
			{
				m_vWidgets[i]->GetImageWidget()->Resolve( "", Colors::AntiqueWhite, "Resources\\Textures\\UI\\Board\\Board.png" );
			}
			m_vWidgets[i]->GetImageWidget()->Update( dt );
		}
#pragma endregion

#pragma region INPUT
		if ( m_vWidgets[i]->GetType() == "Input" )
		{
			static XMFLOAT2 controlTextOffsetPlaceholder = { -20.0f, -27.5f };
			static XMFLOAT2 controlTextOffset = { -7.5f, -7.5f };

			if ( m_vWidgets[i]->GetAction() == "Up Input" )
			{
				m_vWidgets[i]->GetInputWidget()->GetIsUsingPlaceholder() ?
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( controlTextOffsetPlaceholder ) :
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( controlTextOffset );

				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetInputWidget()->Resolve( "W", m_sKeys, Colors::White, m_textures, m_mouseData, i, FontSize::LARGE );
					std::string* inputText = new std::string( m_vWidgets[i]->GetInputWidget()->GetCurrentText() );
					EventSystem::Instance()->AddEvent( EVENTID::KeyInputUpdate_Up, static_cast<void*>( inputText ) );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "Left Input" )
			{
				m_vWidgets[i]->GetInputWidget()->GetIsUsingPlaceholder() ?
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( controlTextOffsetPlaceholder ) :
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( controlTextOffset );

				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetInputWidget()->Resolve( "A", m_sKeys, Colors::White, m_textures, m_mouseData, i, FontSize::LARGE );
					std::string* inputText = new std::string( m_vWidgets[i]->GetInputWidget()->GetCurrentText() );
					EventSystem::Instance()->AddEvent( EVENTID::KeyInputUpdate_Left, static_cast<void*>( inputText ) );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "Down Input" )
			{
				m_vWidgets[i]->GetInputWidget()->GetIsUsingPlaceholder() ?
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( controlTextOffsetPlaceholder ) :
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( controlTextOffset );

				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetInputWidget()->Resolve( "S", m_sKeys, Colors::White, m_textures, m_mouseData, i, FontSize::LARGE );
					std::string* inputText = new std::string( m_vWidgets[i]->GetInputWidget()->GetCurrentText() );
					EventSystem::Instance()->AddEvent( EVENTID::KeyInputUpdate_Down, static_cast<void*>( inputText ) );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "Right Input" )
			{
				m_vWidgets[i]->GetInputWidget()->GetIsUsingPlaceholder() ?
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( controlTextOffsetPlaceholder ) :
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( controlTextOffset );

				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetInputWidget()->Resolve( "D", m_sKeys, Colors::White, m_textures, m_mouseData, i, FontSize::LARGE );
					std::string* inputText = new std::string( m_vWidgets[i]->GetInputWidget()->GetCurrentText() );
					EventSystem::Instance()->AddEvent( EVENTID::KeyInputUpdate_Right, static_cast<void*>( inputText ) );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "Dash Input" )
			{
				m_vWidgets[i]->GetInputWidget()->GetIsUsingPlaceholder() ?
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( { controlTextOffsetPlaceholder.x - 25.0f, controlTextOffsetPlaceholder.y } ) :
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( controlTextOffset );

				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetInputWidget()->Resolve( "SPACE", m_sKeys, Colors::White, m_textures, m_mouseData, i, FontSize::LARGE );
					std::string* inputText = new std::string( m_vWidgets[i]->GetInputWidget()->GetCurrentText() );
					EventSystem::Instance()->AddEvent( EVENTID::KeyInputUpdate_Dash, static_cast<void*>( inputText ) );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "Interact Input" )
			{
				m_vWidgets[i]->GetInputWidget()->GetIsUsingPlaceholder() ?
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( controlTextOffsetPlaceholder ) :
					m_vWidgets[i]->GetInputWidget()->SetTextOffset( controlTextOffset );

				m_vWidgets[i]->SetIsHidden( m_eTabsState == Tabs::Controls ? false : true );
				if ( !m_vWidgets[i]->GetIsHidden() )
				{
					m_vWidgets[i]->GetInputWidget()->Resolve( "E", m_sKeys, Colors::White, m_textures, m_mouseData, i, FontSize::LARGE );
					std::string* inputText = new std::string( m_vWidgets[i]->GetInputWidget()->GetCurrentText() );
					EventSystem::Instance()->AddEvent( EVENTID::KeyInputUpdate_Interact, static_cast<void*>( inputText ) );
				}
			}
			if ( m_vWidgets[i]->GetAction() == "Player Name" )
			{
				// Input that allows the user to enter their name
			}
			if ( m_vWidgets[i]->GetAction() == "" )
			{
				if ( !m_vWidgets[i]->GetIsHidden() )
					m_vWidgets[i]->GetInputWidget()->Resolve( "", m_sKeys, Colors::White, m_textures, m_mouseData, i );
			}
			m_vWidgets[i]->GetInputWidget()->Update( dt );
		}
#pragma endregion

#pragma region PAGE_SLIDERS
		if ( m_vWidgets[i]->GetType() == "Page Slider" )
		{
			m_vWidgets[i]->SetPosition( { m_vScreenSize.x - 30.0f, m_vScreenSize.y * 0.2f } );
			m_vWidgets[i]->SetSize( { 30.0f, m_vScreenSize.y * 0.6f } );

			m_vWidgets[i]->GetPageSliderWidget()->Resolve( Colour( 10.0f, 10.0f, 10.0f ), Colour( 60.0f, 60.0f, 60.0f ), m_mouseData );
			m_vWidgets[i]->GetPageSliderWidget()->Update( dt );

			m_fCurrentY = ( m_vScreenSize.y * 0.2f ) - m_vWidgets[i]->GetPageSliderWidget()->GetPagePos();

			if ( m_bUpdatePageSlider )
			{
				m_vWidgets[i]->GetPageSliderWidget()->SetPageSizeY( m_vScreenSize.y );
				m_bUpdatePageSlider = false;
			}
		}
#pragma endregion
	}

	// Update render box
	if ( m_bUsingPageSlider )
	{
		m_fBoxPos = { 0.0f, m_vScreenSize.y * 0.1f };
		m_fBoxSize = { m_vScreenSize.x, m_vScreenSize.y * 0.6f };
	}
	else
	{
		m_fBoxPos = { 0.0f, 0.0f };
		m_fBoxSize = { m_vScreenSize.x, m_vScreenSize.y };
	}

	if ( !m_mouseData.Hover )
		EventSystem::Instance()->AddEvent( EVENTID::CursorUpdate_Normal );
}

void UIScreen::Draw( VertexShader& vtx, PixelShader& pix, XMMATRIX worldOrtho, TextRenderer* textRenderer )
{
	for ( unsigned int i = 0; i < m_vWidgets.size(); i++ )
	{
		for ( unsigned int j = 0; j < m_vWidgets.size(); j++ )
		{
			if ( m_vWidgets[j]->GetZIndex() == i )
			{
				if ( !m_vWidgets[j]->GetIsHidden() )
				{
					Shaders::BindShaders( m_pContext.Get(), vtx, pix );
					if ( m_vWidgets[j]->GetType() == "Button" )
					{
						RENDER_IF_IN_BOX( m_vWidgets[j]->GetButtonWidget()->GetTransform()->GetPosition().y, m_fBoxPos.y, m_fBoxSize.y,
							m_vWidgets[j]->GetButtonWidget()->Draw( m_pDevice.Get(), m_pContext.Get(), worldOrtho, textRenderer ) );
						Shaders::BindShaders( m_pContext.Get(), vtx, pix );
						break;
					}
					else if ( m_vWidgets[j]->GetType() == "Colour Block" )
					{
						RENDER_IF_IN_BOX( m_vWidgets[j]->GetColourBlockWidget()->GetTransform()->GetPosition().y, m_fBoxPos.y, m_fBoxSize.y,
							m_vWidgets[j]->GetColourBlockWidget()->Draw( m_pDevice.Get(), m_pContext.Get(), worldOrtho ) );
						Shaders::BindShaders( m_pContext.Get(), vtx, pix );
						break;
					}
					else if ( m_vWidgets[j]->GetType() == "Data Slider" )
					{
						RENDER_IF_IN_BOX( m_vWidgets[j]->GetDataSliderWidget()->GetTransform()->GetPosition().y, m_fBoxPos.y, m_fBoxSize.y,
							m_vWidgets[j]->GetDataSliderWidget()->Draw( m_pDevice.Get(), m_pContext.Get(), worldOrtho ) );
						Shaders::BindShaders( m_pContext.Get(), vtx, pix );
						break;
					}
					else if ( m_vWidgets[j]->GetType() == "Drop Down" )
					{
						RENDER_IF_IN_BOX( m_vWidgets[j]->GetDropDownWidget()->GetTransform()->GetPosition().y, m_fBoxPos.y, m_fBoxSize.y,
							m_vWidgets[j]->GetDropDownWidget()->Draw( m_pDevice.Get(), m_pContext.Get(), worldOrtho, textRenderer, vtx, pix ) );
						Shaders::BindShaders( m_pContext.Get(), vtx, pix );
						break;
					}
					else if ( m_vWidgets[j]->GetType() == "Energy Bar" )
					{
						RENDER_IF_IN_BOX( m_vWidgets[j]->GetEnergyBarWidget()->GetTransform()->GetPosition().y, m_fBoxPos.y, m_fBoxSize.y,
							m_vWidgets[j]->GetEnergyBarWidget()->Draw( m_pDevice.Get(), m_pContext.Get(), worldOrtho ) );
						Shaders::BindShaders( m_pContext.Get(), vtx, pix );
						break;
					}
					else if ( m_vWidgets[j]->GetType() == "Image" )
					{
						RENDER_IF_IN_BOX( m_vWidgets[j]->GetImageWidget()->GetTransform()->GetPosition().y, m_fBoxPos.y, m_fBoxSize.y,
							m_vWidgets[j]->GetImageWidget()->Draw( m_pDevice.Get(), m_pContext.Get(), worldOrtho, textRenderer ) );
						Shaders::BindShaders( m_pContext.Get(), vtx, pix );
						break;
					}
					else if ( m_vWidgets[j]->GetType() == "Input" )
					{
						RENDER_IF_IN_BOX( m_vWidgets[j]->GetInputWidget()->GetTransform()->GetPosition().y, m_fBoxPos.y, m_fBoxSize.y,
							m_vWidgets[j]->GetInputWidget()->Draw( m_pDevice.Get(), m_pContext.Get(), worldOrtho, textRenderer ) );
						Shaders::BindShaders( m_pContext.Get(), vtx, pix );
						break;
					}
					else if ( m_vWidgets[j]->GetType() == "Page Slider" )
					{
						RENDER_IF_IN_BOX( m_vWidgets[j]->GetPageSliderWidget()->GetTransform()->GetPosition().y, m_fBoxPos.y, m_fBoxSize.y,
							m_vWidgets[j]->GetPageSliderWidget()->Draw( m_pDevice.Get(), m_pContext.Get(), worldOrtho ) );
						Shaders::BindShaders( m_pContext.Get(), vtx, pix );
						break;
					}
				}
			}
		}
	}
}

void UIScreen::AddToEvent() noexcept
{
	EventSystem::Instance()->AddClient( EVENTID::CharInput, this );
#if _DEBUG
	EventSystem::Instance()->AddClient( EVENTID::ImGuiMousePosition, this );
#endif
	EventSystem::Instance()->AddClient( EVENTID::MousePosition, this );
	EventSystem::Instance()->AddClient( EVENTID::LeftMouseClick, this );
	EventSystem::Instance()->AddClient( EVENTID::LeftMouseRelease, this );
	EventSystem::Instance()->AddClient( EVENTID::RightMouseClick, this );
	EventSystem::Instance()->AddClient( EVENTID::RightMouseRelease, this );
	EventSystem::Instance()->AddClient( EVENTID::MiddleMouseClick, this );
	EventSystem::Instance()->AddClient( EVENTID::MiddleMouseRelease, this );
	EventSystem::Instance()->AddClient( EVENTID::WindowSizeChangeEvent, this );
	EventSystem::Instance()->AddClient(EVENTID::ChangePhase, this);
	EventSystem::Instance()->AddClient( EVENTID::EnemyMaxHealth, this );
	EventSystem::Instance()->AddClient( EVENTID::EnemyCurrentHealth, this );
}

void UIScreen::RemoveFromEvent() noexcept
{
	EventSystem::Instance()->RemoveClient( EVENTID::CharInput, this );
#if _DEBUG
	EventSystem::Instance()->RemoveClient( EVENTID::ImGuiMousePosition, this );
#endif
	EventSystem::Instance()->RemoveClient( EVENTID::MousePosition, this );
	EventSystem::Instance()->RemoveClient( EVENTID::LeftMouseClick, this );
	EventSystem::Instance()->RemoveClient( EVENTID::LeftMouseRelease, this );
	EventSystem::Instance()->RemoveClient( EVENTID::RightMouseClick, this );
	EventSystem::Instance()->RemoveClient( EVENTID::RightMouseRelease, this );
	EventSystem::Instance()->RemoveClient( EVENTID::MiddleMouseClick, this );
	EventSystem::Instance()->RemoveClient( EVENTID::MiddleMouseRelease, this );
	EventSystem::Instance()->RemoveClient( EVENTID::WindowSizeChangeEvent, this );
	EventSystem::Instance()->RemoveClient(EVENTID::ChangePhase, this);
	EventSystem::Instance()->RemoveClient( EVENTID::EnemyMaxHealth, this );
	EventSystem::Instance()->RemoveClient( EVENTID::EnemyCurrentHealth, this );
}


void UIScreen::HandleEvent( Event* event )
{
	switch ( event->GetEventID() )
	{
	case EVENTID::CharInput: { m_sKeys = *(std::string*)event->GetData(); } break;
	case EVENTID::LeftMouseClick: { m_mouseData.LPress = true; } break;
	case EVENTID::LeftMouseRelease: { m_mouseData.LPress = false; } break;
	case EVENTID::RightMouseClick: { m_mouseData.RPress = true; } break;
	case EVENTID::RightMouseRelease: { m_mouseData.RPress = false; } break;
	case EVENTID::MiddleMouseClick: { m_mouseData.MPress = true; } break;
	case EVENTID::MiddleMouseRelease: { m_mouseData.MPress = false; } break;
	case EVENTID::EnemyMaxHealth: {
		m_fMaxHealthPtr = static_cast<float*>(event->GetData());
	} break;
	case EVENTID::EnemyCurrentHealth: {
		m_fCurrentHealthPtr = static_cast<float*>(event->GetData());
	} break;
#if _DEBUG
	case EVENTID::ImGuiMousePosition:
	{
		if ( !g_bDebug ) return;
		Vector2f mousePos = *(Vector2f*)event->GetData();
		m_mouseData.Pos = XMFLOAT2( mousePos.x, mousePos.y );
	}
	break;
#endif
	case EVENTID::MousePosition:
	{
#if _DEBUG
		if ( g_bDebug ) return;
#endif
		m_mouseData.Pos = *(XMFLOAT2*)event->GetData();
	}
	break;
	case EVENTID::WindowSizeChangeEvent:
	{
		m_vScreenSize = *static_cast<XMFLOAT2*>( event->GetData() );
		m_bUpdatePageSlider = true;
	}
	break;
	}
}