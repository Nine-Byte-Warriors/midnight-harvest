#include "stdafx.h"
#include "Projectile.h"
#include "Graphics.h"

Projectile::Projectile(float fSpeed, std::string type, float fLifeTime)
{
	m_vSpawnPosition = Vector2f();
	m_vTargetPosition = Vector2f();
	m_vDirection = Vector2f();
	m_vOffSet = Vector2f();

	m_fSpeed = fSpeed;
	m_fLifeTime = 0.0f;
	m_fMaxLifeTime = fLifeTime;
	m_fDelay = 0.0f;

	m_bFixedDirection = true;
	m_fAmplitude = 0.0f;
	m_fFrequency = 0.0f;
	
	m_sprite = std::make_shared<Sprite>();
	m_transform = std::make_shared<Transform>( m_sprite );
	m_physics = std::make_shared<Physics>(m_transform);
	m_collider = std::make_shared<CircleCollider>(m_transform, m_sprite, true, 2, type, 32);

	if(type == "PlayerProjectile")
	m_collider->SetLayer(LayerNo::PlayerProjectile);
	else if(type == "EnemyProjectile")
	m_collider->SetLayer(LayerNo::EnemyProjectile);

	m_owner = ProjectileOwner::None;
}

void Projectile::Initialize(const Graphics& gfx, ConstantBuffer<Matrices>& mat, Sprite::Type type)
{
	m_sprite->Initialize(gfx.GetDevice(), gfx.GetContext(), type, mat);
	m_transform->SetPositionInit(0.0f, 0.0f);
}

void Projectile::Initialize(const Graphics& gfx, ConstantBuffer<Matrices>& mat, const std::string& sSpritePath)
{
	m_sprite->Initialize(gfx.GetDevice(), gfx.GetContext(), sSpritePath, mat);
	m_transform->SetPositionInit(0.0f, 0.0f);
}

void Projectile::Initialize(const Graphics& gfx, ConstantBuffer<Matrices>& mat, const std::string& sSpritePath, Vector2f vSize)
{
	m_sprite->Initialize(gfx.GetDevice(), gfx.GetContext(), sSpritePath, mat);
	m_transform->SetPositionInit(0.0f, 0.0f);

	float fWidth = vSize.x == 0.0f ? m_sprite->GetWidth() : vSize.x;
	float fHeight = vSize.y == 0.0f ? m_sprite->GetHeight() : vSize.y;
	m_sprite->SetWidthHeight( fWidth, fHeight );
}

void Projectile::Update(const float dt)
{
	if (!IsAlive())
	{
		SetOwner(ProjectileOwner::None);
		return;
	}
	
	m_fDelay -= dt;
	if (m_fDelay > 0.0f)
		return;

	m_fLifeTime -= dt;

	if (m_fAmplitude == 0.0f || m_fFrequency == 0.0f)
		m_physics->AddForce(m_vDirection.Multiply(m_fSpeed));
	else
		CalcDirection();

	m_sprite->Update(dt);
	m_physics->Update(dt);
	m_transform->Update();
}

void Projectile::Draw(ID3D11DeviceContext* context, XMMATRIX orthoMatrix)
{
	if (!IsAlive())
		return;

	if (m_fDelay > 0.0f)
		return;
	
	m_sprite->UpdateBuffers(context);
	m_sprite->Draw(m_transform->GetWorldMatrix(), orthoMatrix);
}

void Projectile::SpawnProjectile(Vector2f vSpawnPosition, Vector2f vTargetPosition, float fLifeTime)
{
	m_fLifeTime = fLifeTime <= 0.0f ? m_fMaxLifeTime : fLifeTime;
	
	m_vDirection = vSpawnPosition
		.DirectionTo(vTargetPosition)
		.Normalised();
	
	m_vAnchorPosition = vSpawnPosition;
	m_transform->SetPosition(m_vAnchorPosition);

	m_physics->ResetForces();
}

void Projectile::SpawnProjectile(Vector2f vSpawnPosition, float fLifeTime)
{
	m_fLifeTime = fLifeTime <= 0.0f	? m_fMaxLifeTime : fLifeTime;

	m_vAnchorPosition = vSpawnPosition.Add(m_vOffSet);
	m_transform->SetPosition(m_vAnchorPosition);

	m_physics->ResetForces();
}

void Projectile::CalcDirection()
{
	float fAngle = m_fAngle + AI_MATH_HALF_PI_F;
	float fCurrentDist = m_fAmplitude * sinf((m_fMaxLifeTime - m_fLifeTime) * m_fFrequency);
	Vector2f vWavePosition = Vector2f(
		cosf(fAngle) * fCurrentDist,
		sinf(fAngle) * fCurrentDist
	);
	m_vAnchorPosition = m_vAnchorPosition.Add(m_vDirection.Multiply(m_fSpeed));
	m_transform->SetPosition(vWavePosition.Add(m_vAnchorPosition));
}