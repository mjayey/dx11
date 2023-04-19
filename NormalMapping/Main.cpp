#include <WinApp.h>
#include <AppUtil.h>
#include <Lights.h>
#include <Camera.h>
#include <GeometryGens.h>
#include <RenderStates.h>
#include "Effects.h"
#include "Inputs.h"

class NormalMappingDemo: public WinApp
{
public:
	NormalMappingDemo(HINSTANCE hInst, std::wstring title = L"Normal Mapping Demo", int width = 800, int height = 600);
	~NormalMappingDemo();

	bool	Init();
	bool	OnResize();
	bool	Update(float timeDelt);
	bool	Render();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	bool	BuildBuffers();
	bool	BuildSRVs();

private:
	ID3D11Buffer	*m_VB;
	ID3D11Buffer	*m_IB;

	ID3D11ShaderResourceView	*m_floorSRV;
	ID3D11ShaderResourceView	*m_floorNormal;

	GeoGen::MeshData	m_floor;

	Lights::DirLight	m_dirLights[3];
	Lights::Material	m_material;

	ID3DX11EffectTechnique	*m_tech;

	Camera		m_camera;
	POINT		m_lastPos;
};

NormalMappingDemo::NormalMappingDemo(HINSTANCE hInst, std::wstring title, int width, int height):WinApp(hInst,title,width,height),
	m_VB(NULL),
	m_IB(NULL),
	m_floorSRV(NULL),
	m_floorNormal(NULL),
	m_tech(NULL)
{
	m_camera.LookAt(XMFLOAT3(0.5f,1.01f,0.5f),XMFLOAT3(-0.7f,0.f,-0.7f),XMFLOAT3(0.f,1.f,0.f));

	//3 directional lights
	//Main Light
	m_dirLights[0].ambient  =	XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_dirLights[0].diffuse  =	XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_dirLights[0].specular =	XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_dirLights[0].dir		 =	XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
	//light2
	m_dirLights[1].ambient  =	XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_dirLights[1].diffuse  =	XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	m_dirLights[1].specular =	XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	m_dirLights[1].dir		 =	XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);
	//light3
	m_dirLights[2].ambient  =	XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_dirLights[2].diffuse  =	XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_dirLights[2].specular =	XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_dirLights[2].dir	     =	XMFLOAT3(0.0f, -0.707f, -0.707f);

	m_material.ambient = XMFLOAT4(0.5f,0.5f,0.5f,1.f);
	m_material.diffuse = XMFLOAT4(1.f,1.f,1.f,1.f);
	m_material.specular = XMFLOAT4(0.3f,0.3f,0.3f,10.f);
	m_material.reflection = XMFLOAT4(0.8f,0.8f,0.8f,0.8f);
}

NormalMappingDemo::~NormalMappingDemo()
{
	SafeRelease(m_VB);
	SafeRelease(m_IB);
	SafeRelease(m_floorSRV);
	SafeRelease(m_floorNormal);
}

bool NormalMappingDemo::Init()
{
	if(!WinApp::Init())
		return false;

	if(!Effects::InitAll(m_d3dDevice))
		return false;
	if(!InputLayouts::InitAll(m_d3dDevice))
		return false;
	if(!RenderStates::InitAll(m_d3dDevice))
		return false;
	if(!BuildBuffers())
		return false;
	if(!BuildSRVs())
		return false;

	m_tech = Effects::fxBasic->fxLight3TexNormalParallaxMappingTech;

	return true;
}

bool NormalMappingDemo::BuildBuffers()
{
	GeoGen::CreateGrid(5.f,5.f,20,20,m_floor);

	D3D11_BUFFER_DESC vDesc = {0};
	vDesc.ByteWidth = sizeof(GeoGen::Vertex) * m_floor.vertices.size();
	vDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vData;
	vData.pSysMem = &m_floor.vertices[0];
	vData.SysMemPitch = 0;
	vData.SysMemSlicePitch = 0;
	if(FAILED(m_d3dDevice->CreateBuffer(&vDesc,&vData,&m_VB)))
	{
		MessageBox(NULL,L"Create Vertex Buffer failed!",L"Error",MB_OK);
		return false;
	}

	D3D11_BUFFER_DESC iDesc = {0};
	iDesc.ByteWidth = sizeof(UINT) * m_floor.indices.size();
	iDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA iData;
	iData.pSysMem = &m_floor.indices[0];
	iData.SysMemPitch = 0;
	iData.SysMemSlicePitch = 0;
	if(FAILED(m_d3dDevice->CreateBuffer(&iDesc,&iData,&m_IB)))
	{
		MessageBox(NULL,L"Create Index Buffer failed!",L"Error",MB_OK);
		return false;
	}

	return true;
}

bool NormalMappingDemo::Update(float delta)
{
	if(KeyDown('A'))
	{
		m_camera.Strafe(-2.f*delta);
	}
	else if(KeyDown('D'))
	{
		m_camera.Strafe(2.f*delta);
	}
	if(KeyDown('W'))
	{
		m_camera.Walk(2.f*delta);
	}
	else if(KeyDown('S'))
	{
		m_camera.Walk(-2.f*delta);
	}

	XMFLOAT3 pos = m_camera.GetPosition();
	pos.y = 1.01f;
	m_camera.SetPosition(pos.x,pos.y,pos.z);

	if(KeyDown('1'))
		m_tech = Effects::fxBasic->fxLight3Tech;
	else if(KeyDown('2'))
		m_tech = Effects::fxBasic->fxLight3NormalMappingTech;
	else if(KeyDown('3'))
		m_tech = Effects::fxBasic->fxLight3NormalParallaxMappingTech;
	else if(KeyDown('4'))
		m_tech = Effects::fxBasic->fxLight3TexTech;
	else if(KeyDown('5'))
		m_tech = Effects::fxBasic->fxLight3TexNormalMappingTech;
	else if(KeyDown('6'))
		m_tech = Effects::fxBasic->fxLight3TexNormalParallaxMappingTech;

	//Update per frame shader variables
	Effects::fxBasic->SetEyePos(m_camera.GetPosition());
	Effects::fxBasic->SetLights(m_dirLights);

	m_camera.UpdateView();

	return true;
}

bool NormalMappingDemo::Render()
{
	m_deviceContext->ClearDepthStencilView(m_depthStencilView,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.f,0);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView,reinterpret_cast<const float*>(&Colors::Black));
	m_deviceContext->IASetInputLayout(InputLayouts::posNormalTagentTex);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	UINT stride = sizeof(GeoGen::Vertex);
	UINT offset = 0;
	m_deviceContext->IASetVertexBuffers(0,1,&m_VB,&stride,&offset);
	m_deviceContext->IASetIndexBuffer(m_IB,DXGI_FORMAT_R32_UINT,0);

	D3DX11_TECHNIQUE_DESC desc;
	m_tech->GetDesc(&desc);
	for(UINT i=0; i<desc.Passes; ++i)
	{
		Effects::fxBasic->SetWorldMatrix(XMMatrixIdentity());
		Effects::fxBasic->SetWorldViewProjMatrix(m_camera.ViewProjection());
		Effects::fxBasic->SetWorldInvTransposeMatrix(XMMatrixIdentity());
		Effects::fxBasic->SetTextureTransform(XMMatrixScaling(2.f,2.f,1.f));
		Effects::fxBasic->SetTextureOffsetScale(XMFLOAT2(0.4f,0.4f));		//Transform the world space offset into texture space offset
		Effects::fxBasic->SetHeightScale(0.08f);			//Scale the height value read from the height map
		Effects::fxBasic->SetMaterial(m_material);
		Effects::fxBasic->SetShaderResource(m_floorSRV);
		Effects::fxBasic->SetNormalMap(m_floorNormal);

		m_tech->GetPassByIndex(i)->Apply(0,m_deviceContext);
		m_deviceContext->DrawIndexed(m_floor.indices.size(),0,0);
	}

	m_swapChain->Present(0,0);

	return true;
}

bool NormalMappingDemo::OnResize()
{
	if(!WinApp::OnResize())
		return false;

	m_camera.SetLens(XM_PI*0.25f,1.f*m_clientWidth/m_clientHeight,1.f,1000.f);

	return true;
}

bool NormalMappingDemo::BuildSRVs()
{
	if(FAILED(D3DX11CreateShaderResourceViewFromFile(m_d3dDevice,L"textures/stones.dds",0,0,&m_floorSRV,0)))
	{
		MessageBox(NULL,L"Create SRV failed!",L"Error",MB_OK);
		return false;
	}
	
	if(FAILED(D3DX11CreateShaderResourceViewFromFile(m_d3dDevice,L"textures/stones_nmap.dds",0,0,&m_floorNormal,0)))
	{
		MessageBox(NULL,L"Create normal map failed!",L"Error",MB_OK);
		return false;
	}

	return true;
}

void NormalMappingDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastPos.x = x;
	m_lastPos.y = y;
	SetCapture(m_hWnd);
}

void NormalMappingDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void NormalMappingDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
	if((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * (x - m_lastPos.x));
		float dy = XMConvertToRadians(0.25f * (y - m_lastPos.y));

		m_camera.Pitch(dy);
		m_camera.RotateY(dx);
	}

	m_lastPos.x = x;
	m_lastPos.y = y;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int cmdShow)
{
	NormalMappingDemo demo(hInstance);

	if(!demo.Init())
		return false;

	return demo.Run();
}