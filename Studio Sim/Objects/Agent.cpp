#include "stdafx.h"
#include "Agent.h"
#include "Camera.h"
#include <dxtk/WICTextureLoader.h>

bool Agent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context,
	float width, float height, std::string spritePath,
	ConstantBuffer<Matrices2D>& cb_vs_matrix_2d)
{
	this->context = context;
	if (context == nullptr) return false;

	this->cb_vs_matrix_2d = &cb_vs_matrix_2d;
	texture = std::make_unique<Texture>(device, spritePath, aiTextureType_DIFFUSE);

	std::vector<Vertex2D> vertexData =
	{
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f } },
		{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f } },
		{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f } },
	};

	std::vector<WORD> indexData =
	{
		0, 1, 2,
		2, 1, 3
	};

	try
	{
		HRESULT hr = vertices.Initialize(device, vertexData.data(), static_cast<UINT>(vertexData.size()));
		COM_ERROR_IF_FAILED(hr, "Failed to initialize vertices for sprite!");

		hr = indices.Initialize(device, indexData.data(), static_cast<UINT>(indexData.size()));
		COM_ERROR_IF_FAILED(hr, "Failed to initialize indices for sprite!");

		hr = m_cbAnimation.Initialize(device, context);
		COM_ERROR_IF_FAILED(hr, "Failed to create 'Animation' constant buffer!");
	}
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}

	SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetScale(width, height);

	InitialiseAILogic();

	return true;
}

// Initialize Colour Texture
bool Agent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, float width, float height, Colour spriteColour, ConstantBuffer<Matrices2D>& cb_vs_matrix_2d)
{
	this->context = context;
	if (context == nullptr) return false;

	this->cb_vs_matrix_2d = &cb_vs_matrix_2d;
	texture = std::make_unique<Texture>(device, spriteColour, aiTextureType_DIFFUSE);

	std::vector<Vertex2D> vertexData =
	{
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f } },
		{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f } },
		{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f } },
	};

	std::vector<WORD> indexData =
	{
		0, 1, 2,
		2, 1, 3
	};

	HRESULT hr = vertices.Initialize(device, vertexData.data(), static_cast<UINT>(vertexData.size()));
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertices for sprite!");

	hr = indices.Initialize(device, indexData.data(), static_cast<UINT>(indexData.size()));
	COM_ERROR_IF_FAILED(hr, "Failed to initialize indices for sprite!");

	SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetScale(width, height);

	InitialiseAILogic();

	return true;
}

void Agent::Draw(XMMATRIX orthoMatrix)
{
	XMMATRIX wvpMatrix = worldMatrix * orthoMatrix;
	cb_vs_matrix_2d->data.wvpMatrix = wvpMatrix;
	cb_vs_matrix_2d->ApplyChanges();
	context->VSSetConstantBuffers(0, 1, cb_vs_matrix_2d->GetAddressOf());
	context->PSSetShaderResources(0, 1, texture->GetTextureResourceViewAddress());

	const UINT offsets = 0;
	context->IASetVertexBuffers(0, 1, vertices.GetAddressOf(), vertices.StridePtr(), &offsets);
	context->IASetIndexBuffer(indices.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->DrawIndexed(indices.IndexCount(), 0, 0);
}

void Agent::UpdateBuffers(ID3D11DeviceContext* context)
{
	m_cbAnimation.data.Width = scale.x;
	m_cbAnimation.data.Height = scale.y;
	m_cbAnimation.data.Rows = 2.0f;
	m_cbAnimation.data.Columns = 2.0f;
	m_cbAnimation.data.FrameX = 0;
	m_cbAnimation.data.FrameY = 0;
	if (!m_cbAnimation.ApplyChanges()) return;
	context->VSSetConstantBuffers(1u, 1u, m_cbAnimation.GetAddressOf());
}

void Agent::UpdateMatrix()
{
	XMFLOAT3 pos = GetPositionFloat3();
	
	worldMatrix = XMMatrixScaling(scale.x, scale.y, 1.0f) *
		XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
		XMMatrixTranslation(pos.x, pos.y, pos.z);
}

void Agent::InitialiseAILogic()
{
	m_fSpeed = 10.0f;
	
	m_pStateMachine = new AILogic::AIStateMachine(this);
	AILogic::AIState* pSeekState = m_pStateMachine->NewState(AILogic::AIStateTypes::Seek);
	pSeekState->SetBounds(1.0f, 0.0f);
	pSeekState->SetActivation(0.0f);
	m_vecStates.push_back(pSeekState);
	
	AILogic::AIState* pIdleState = m_pStateMachine->NewState(AILogic::AIStateTypes::Idle);
	pIdleState->SetActivation(0.0f);
	m_vecStates.push_back(pIdleState);

	AILogic::AIState* pFleeState = m_pStateMachine->NewState(AILogic::AIStateTypes::Flee);
	pFleeState->SetBounds(1.0f, 0.0f);
	pFleeState->SetActivation(1.0f);
	m_vecStates.push_back(pFleeState);
	
	AddToEvent();
}

void Agent::Update(float dt)
{
	m_pStateMachine->Clear();
	
	for (AILogic::AIState* pState : m_vecStates)
		m_pStateMachine->AddState(pState);
	
	m_pStateMachine->UpdateMachine(dt);
	
	UpdateMatrix();
}

void Agent::HandleEvent(Event* event)
{
	switch (event->GetEventID())
	{
	case EVENTID::PlayerPosition:
		m_vTargetPos = *(Vector2f*)event->GetData();
		break;
	default:
		break;
	}
}
