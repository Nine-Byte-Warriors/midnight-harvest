#pragma once
#ifndef ENERGYBAR_WIDGET_H
#define ENERGYBAR_WIDGET_H

#include "Widget.h"
#include "Transform.h"

class EnergyBar_Widget : public Widget
{
public:
    EnergyBar_Widget();
    EnergyBar_Widget( const std::vector<std::string>& textures, float fraction, XMFLOAT2 pos, XMFLOAT2 size );
    ~EnergyBar_Widget();

	void Initialize( ID3D11Device* device, ID3D11DeviceContext* context, ConstantBuffer<Matrices>& mat );
    void Update( const float dt );
    void Draw( ID3D11Device* device, ID3D11DeviceContext* context, XMMATRIX worldOrtho );
    void Resolve( const std::vector<std::string>& textures, float fraction, XMFLOAT2 pos, XMFLOAT2 size );

	int GetCurrentPercent() const noexcept{ return m_iCurrentPercent; }

private:
    int m_iCurrentPercent;
    float m_fCurrentFraction;
    XMFLOAT2 m_vPosition, m_vSize;

	// Background
    std::string m_textureBack;
    std::shared_ptr<Sprite> m_spriteBack;
    std::shared_ptr<Transform> m_transformBack;

	// Moving bar
    std::string m_textureBar;
    std::shared_ptr<Sprite> m_spriteBar;
    std::shared_ptr<Transform> m_transformBar;
	
	// Foreground
    std::string m_textureFront;
    std::shared_ptr<Sprite> m_spriteFront;
    std::shared_ptr<Transform> m_transformFront;
};

#endif