#include <WinApp.h>
#include <AppUtil.h>
#include <GeometryGens.h>
#include <RenderStates.h>
#include <Lights.h>
#include <Camera.h>
#include "Effects.h"
#include "Inputs.h"

//Vertex format used in sky box rendering
struct PosVertex
{
	XMFLOAT3	pos;
};

//Sky box with reflection
class DynamicCubeMapping: public WinApp
{
public:
	DynamicCubeMapping(HINSTANCE hInst, std::wstring title = L"D3D11 Cube Mapping", int width = 800, int height = 600);
	~DynamicCubeMapping();

	bool	Init();							//Initialization
	bool	OnResize();						//Window size changed
	bool	Update(float timeDelt);			//Update for each frame
	bool	Render();						//Scene rendering for each frame

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	bool BuildDynamicCubeMappingViews();
	void BuildDynamicCameras();
	bool BuildBuffers();
	bool BuildCubeMap();

private:
	ID3D11Buffer	*m_VBSky;
	ID3D11Buffer	*m_IBSky;

	ID3D11Buffer	*m_VBObjects;
	ID3D11Buffer	*m_IBObjects;

	ID3D11ShaderResourceView	*m_cubeMapSRV;
	ID3D11ShaderResourceView	*m_boxSRV;

	UINT	m_cubeMapWidth;
	UINT	m_cubeMapHeight;
	ID3D11RenderTargetView		*m_dynamicRTV[6];
	ID3D11DepthStencilView		*m_dynamicDSV;
	ID3D11ShaderResourceView	*m_dynamicSRV;
	D3D11_VIEWPORT				m_dynamicViewport;

	Camera						m_dynamicCameras[6];

	GeoGen::MeshData	m_skySphere;
	GeoGen::MeshData	m_sphere;
	GeoGen::MeshData	m_box;

	UINT	m_sphereVStart, m_sphereIStart;
	UINT	m_boxVStart, m_boxIStart;

	XMFLOAT4X4		m_worldSphere;
	XMFLOAT4X4		m_invWorldTranspose;

	XMFLOAT4X4		m_worldBox;
	XMFLOAT4X4		m_invWorldTransposeBox;

	Lights::Material	m_material;

	Lights::DirLight	m_dirLights[3];

	Camera		m_camera;
	
	POINT		m_lastPos;
};

DynamicCubeMapping::DynamicCubeMapping(HINSTANCE hInst, std::wstring title, int width, int height):WinApp(hInst,title,width,height),
	m_VBSky(NULL),
	m_IBSky(NULL),
	m_VBObjects(NULL),
	m_IBObjects(NULL),
	m_cubeMapSRV(NULL),
	m_boxSRV(NULL),
	m_cubeMapWidth(256),
	m_cubeMapHeight(256),
	m_dynamicDSV(NULL),
	m_dynamicSRV(NULL)
{
	m_camera.SetPosition(0.f,0.f,-3.f);

	for(UINT i=0; i<6; ++i)
	{
		m_dynamicRTV[i] = NULL;
	}

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
	m_material.specular = XMFLOAT4(0.3f,0.3f,0.3f,30.f);
	m_material.reflection = XMFLOAT4(0.8f,0.8f,0.8f,0.8f);

	XMStoreFloat4x4(&m_worldSphere,XMMatrixIdentity());
	XMStoreFloat4x4(&m_invWorldTranspose,XMMatrixIdentity());
}

DynamicCubeMapping::~DynamicCubeMapping()
{
	SafeRelease(m_VBSky);
	SafeRelease(m_IBSky);
	SafeRelease(m_VBObjects);
	SafeRelease(m_IBObjects);
	SafeRelease(m_cubeMapSRV);
	SafeRelease(m_boxSRV);
	SafeRelease(m_dynamicDSV);
	SafeRelease(m_dynamicSRV);
	for(UINT i=0; i<6; ++i)
	{
		SafeRelease(m_dynamicRTV[i]);
	}

	Effects::ReleaseAll();
	InputLayouts::ReleaseAll();
	RenderStates::ReleaseAll();
}

bool DynamicCubeMapping::Init()
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
	if(!BuildDynamicCubeMappingViews())
		return false;
	if(!BuildCubeMap())
		return false;

	BuildDynamicCameras();

	return true;
}

bool DynamicCubeMapping::BuildBuffers()
{
	//For sky
	GeoGen::CreateSphere(100.f,30,30,m_skySphere);

	D3D11_BUFFER_DESC descSky = {0};
	descSky.ByteWidth = sizeof(PosVertex) * m_skySphere.vertices.size();
	descSky.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	descSky.Usage = D3D11_USAGE_DEFAULT;

	std::vector<PosVertex> verticesSky(m_skySphere.vertices.size());
	for(UINT i=0; i<m_skySphere.vertices.size(); ++i)
	{
		verticesSky[i].pos = m_skySphere.vertices[i].pos;
	}

	D3D11_SUBRESOURCE_DATA vDataSky;
	vDataSky.pSysMem = &verticesSky[0];
	vDataSky.SysMemPitch = 0;
	vDataSky.SysMemSlicePitch = 0;
	if(FAILED(m_d3dDevice->CreateBuffer(&descSky,&vDataSky,&m_VBSky)))
	{
		MessageBox(NULL,L"Create Vertex Buffer failed!",L"Error",MB_OK);
		return false;
	}

	D3D11_BUFFER_DESC iDescSky = {0};
	iDescSky.ByteWidth = sizeof(UINT) * m_skySphere.indices.size();
	iDescSky.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iDescSky.Usage = D3D11_USAGE_DEFAULT;
	std::vector<UINT> indicesSky(m_skySphere.indices.size());
	for(UINT i=0; i<m_skySphere.indices.size(); ++i)
	{
		indicesSky[i] = m_skySphere.indices[i];
	}
	D3D11_SUBRESOURCE_DATA iDataSky;
	iDataSky.pSysMem = &indicesSky[0];
	iDataSky.SysMemPitch = 0;
	iDataSky.SysMemSlicePitch = 0;
	if(FAILED(m_d3dDevice->CreateBuffer(&iDescSky,&iDataSky,&m_IBSky)))
	{
		MessageBox(NULL,L"Create Index Buffer failed!",L"Error",MB_OK);
		return false;
	}

	//For sphere and box
	GeoGen::CreateSphere(1.0f,30,30,m_sphere);
	m_sphereVStart = m_sphereIStart = 0;
	GeoGen::CreateBox(1.f,1.f,1.f,m_box);
	m_boxVStart = m_sphere.vertices.size();
	m_boxIStart = m_sphere.indices.size();

	D3D11_BUFFER_DESC descObjects = {0};
	descObjects.ByteWidth = sizeof(Vertex::Basic32) * (m_sphere.vertices.size() + m_box.vertices.size());
	descObjects.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	descObjects.Usage = D3D11_USAGE_DEFAULT;

	std::vector<Vertex::Basic32> verticesObjects(m_sphere.vertices.size() + m_box.vertices.size());
	for(UINT i=0; i<m_sphere.vertices.size(); ++i)
	{
		verticesObjects[i].pos = m_sphere.vertices[i].pos;
		verticesObjects[i].normal = m_sphere.vertices[i].normal;
		verticesObjects[i].tex = m_sphere.vertices[i].tex;
	}
	for(UINT i=0; i<m_box.vertices.size(); ++i)
	{
		verticesObjects[i + m_boxVStart].pos = m_box.vertices[i].pos;
		verticesObjects[i + m_boxVStart].normal = m_box.vertices[i].normal;
		verticesObjects[i + m_boxVStart].tex = m_box.vertices[i].tex;
	}

	D3D11_SUBRESOURCE_DATA vDataObjects;
	vDataObjects.pSysMem = &verticesObjects[0];
	vDataObjects.SysMemPitch = 0;
	vDataObjects.SysMemSlicePitch = 0;
	if(FAILED(m_d3dDevice->CreateBuffer(&descObjects,&vDataObjects,&m_VBObjects)))
	{
		MessageBox(NULL,L"Create Vertex Buffer failed!",L"Error",MB_OK);
		return false;
	}

	D3D11_BUFFER_DESC iDescObjects = {0};
	iDescObjects.ByteWidth = sizeof(UINT) * (m_sphere.indices.size() + m_box.indices.size());
	iDescObjects.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iDescObjects.Usage = D3D11_USAGE_DEFAULT;
	std::vector<UINT> indicesObjects(m_sphere.indices.size() + m_box.indices.size());
	for(UINT i=0; i<m_sphere.indices.size(); ++i)
	{
		indicesObjects[i] = m_sphere.indices[i];
	}
	for(UINT i=0; i<m_box.indices.size(); ++i)
	{
		indicesObjects[i + m_boxIStart] = m_box.indices[i];
	}
	D3D11_SUBRESOURCE_DATA iDataObjects;
	iDataObjects.pSysMem = &indicesObjects[0];
	iDataObjects.SysMemPitch = 0;
	iDataObjects.SysMemSlicePitch = 0;
	if(FAILED(m_d3dDevice->CreateBuffer(&iDescObjects,&iDataObjects,&m_IBObjects)))
	{
		MessageBox(NULL,L"Create Index Buffer failed!",L"Error",MB_OK);
		return false;
	}
	return true;
}

bool DynamicCubeMapping::BuildCubeMap()
{
	if(FAILED(D3DX11CreateShaderResourceViewFromFile(m_d3dDevice,L"textures/snowcube1024.dds",0,0,&m_cubeMapSRV,0)))
	{
		MessageBox(NULL,L"Create Cube Map from file failed!",L"Error",MB_OK);
		return false;
	}
	if(FAILED(D3DX11CreateShaderResourceViewFromFile(m_d3dDevice,L"textures/Wood.dds",0,0,&m_boxSRV,0)))
	{
		MessageBox(NULL,L"Create wood srv from file failed!",L"Error",MB_OK);
		return false;
	}
	return true;
}

bool DynamicCubeMapping::BuildDynamicCubeMappingViews()
{
	//Create the dynamic cube map
	D3D11_TEXTURE2D_DESC cubeMapDesc;
	cubeMapDesc.Width = m_cubeMapWidth;
	cubeMapDesc.Height = m_cubeMapHeight;
	cubeMapDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	cubeMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	cubeMapDesc.ArraySize = 6;
	cubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeMapDesc.CPUAccessFlags = 0;
	cubeMapDesc.MipLevels = 0;
	cubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	cubeMapDesc.SampleDesc.Count = 1;
	cubeMapDesc.SampleDesc.Quality = 0;
	
	ID3D11Texture2D *cubeMap(NULL);
	if(FAILED(m_d3dDevice->CreateTexture2D(&cubeMapDesc,0,&cubeMap)))
	{
		MessageBox(NULL,L"Create dynamic Cube Map failed!",L"Error",MB_OK);
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = cubeMapDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;
	
	for(UINT i=0; i<6; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		if(FAILED(m_d3dDevice->CreateRenderTargetView(cubeMap,&rtvDesc,&m_dynamicRTV[i])))
		{
			SafeRelease(cubeMap);
			MessageBox(NULL,L"Create dynamic Cube Map  rtv failed!",L"Error",MB_OK);
			return false;
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = cubeMapDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = -1;
	srvDesc.TextureCube.MostDetailedMip = 0;
	if(FAILED(m_d3dDevice->CreateShaderResourceView(cubeMap,&srvDesc,&m_dynamicSRV)))
	{
		SafeRelease(cubeMap);
		MessageBox(NULL,L"Create dynamic Cube Map srv failed!",L"Error",MB_OK);
		return false;
	}

	SafeRelease(cubeMap);
	
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Width = m_cubeMapWidth;
	dsDesc.Height = m_cubeMapHeight;
	dsDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsDesc.ArraySize = 1;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;
	dsDesc.MipLevels = 1;
	
	ID3D11Texture2D *depthStencilBuffer(NULL);
	if(FAILED(m_d3dDevice->CreateTexture2D(&dsDesc,0,&depthStencilBuffer)))
	{
		SafeRelease(depthStencilBuffer);
		MessageBox(NULL,L"Create dynamic depth stencil buffer failed!",L"Error",MB_OK);
		return false;
	}
	if(FAILED(m_d3dDevice->CreateDepthStencilView(depthStencilBuffer,0,&m_dynamicDSV)))
	{
		SafeRelease(depthStencilBuffer);
		MessageBox(NULL,L"Create dynamic DSV failed!",L"Error",MB_OK);
		return false;
	}

	SafeRelease(depthStencilBuffer);

	m_dynamicViewport.Width = static_cast<float>(m_cubeMapWidth);
	m_dynamicViewport.Height = static_cast<float>(m_cubeMapHeight);
	m_dynamicViewport.TopLeftX = 0.f;
	m_dynamicViewport.TopLeftY = 0.f;
	m_dynamicViewport.MinDepth = 0.f;
	m_dynamicViewport.MaxDepth = 1.f;

	return true;
}

void DynamicCubeMapping::BuildDynamicCameras()
{
	XMFLOAT3 ups[6] = 
	{
		XMFLOAT3( 0.f, 1.f, 0.f),
		XMFLOAT3( 0.f, 1.f, 0.f),
		XMFLOAT3( 0.f, 0.f,-1.f),
		XMFLOAT3( 0.f, 0.f, 1.f),
		XMFLOAT3( 0.f, 1.f, 0.f),
		XMFLOAT3( 0.f, 1.f, 0.f)
	};

	XMFLOAT3 targets[6] = 
	{
		XMFLOAT3( 1.f, 0.f, 0.f),
		XMFLOAT3(-1.f, 0.f, 0.f),
		XMFLOAT3( 0.f, 1.f, 0.f),
		XMFLOAT3( 0.f,-1.f, 0.f),
		XMFLOAT3( 0.f, 0.f, 1.f),
		XMFLOAT3( 0.f, 0.f,-1.f)
	};

	for(UINT i=0; i<6; ++i)
	{
		m_dynamicCameras[i].LookAt(XMFLOAT3(0.f,0.f,0.f),targets[i],ups[i]);
		m_dynamicCameras[i].SetLens(XM_PI*0.5f,1.f,1.f,1000.f);
		m_dynamicCameras[i].UpdateView();
	}
}

bool DynamicCubeMapping::OnResize()
{
	if(!WinApp::OnResize())
		return false;

	m_camera.SetLens(XM_PI*0.25f,1.f*m_clientWidth/m_clientHeight,1.f,1000.f);

	return true;
}

bool DynamicCubeMapping::Update(float delta)
{
	if(KeyDown('A'))
	{
		m_camera.Strafe(-6.f*delta);
	}
	else if(KeyDown('D'))
	{
		m_camera.Strafe(6.f*delta);
	}
	if(KeyDown('W'))
	{
		m_camera.Walk(6.f*delta);
	}
	else if(KeyDown('S'))
	{
		m_camera.Walk(-6.f*delta);
	}

	static float angle1(0.f), angle2(0.f);
	angle1 += XM_PI*0.5f*delta;
	angle2 += XM_PI*0.5f*delta;
	if(angle1 > XM_PI*2.f)
		angle1 = 0;
	if(angle2 > XM_PI*2.f)
		angle2 = 0;
	
	XMMATRIX worldBox = XMMatrixRotationY(angle2) * XMMatrixTranslation(3.f*cos(angle1),0.f,3.f*sin(angle1));
	XMStoreFloat4x4(&m_worldBox,worldBox);
	XMStoreFloat4x4(&m_invWorldTransposeBox,InverseTranspose(worldBox));

	//Update per frame shader variables
	Effects::fxBasic->SetLights(m_dirLights);
	Effects::fxBasic->SetEyePos(m_camera.GetPosition());

	m_camera.UpdateView();

	return true;
}

bool DynamicCubeMapping::Render()
{
	//First, render the scene(except the sphere) into texture to generate cube 
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11RenderTargetView *rtv[1] = {0};
	Camera *tmpCamera(NULL);
	m_deviceContext->RSSetViewports(1,&m_dynamicViewport);
	for(UINT i=0; i<6; ++i)
	{
		rtv[0] = m_dynamicRTV[i];
		m_deviceContext->OMSetRenderTargets(1,&rtv[0],m_dynamicDSV);
		m_deviceContext->ClearRenderTargetView(rtv[0],reinterpret_cast<const float*>(&Colors::Silver));
		m_deviceContext->ClearDepthStencilView(m_dynamicDSV,D3D11_CLEAR_DEPTH,1.0f,0); 
		tmpCamera = &m_dynamicCameras[i];

		m_deviceContext->IASetInputLayout(InputLayouts::pos);
		UINT stride1 = sizeof(PosVertex);
		UINT offset1 = 0;
		m_deviceContext->IASetVertexBuffers(0,1,&m_VBSky,&stride1,&offset1);
		m_deviceContext->IASetIndexBuffer(m_IBSky,DXGI_FORMAT_R32_UINT,0);

		ID3DX11EffectTechnique *tech = Effects::fxSkyBox->fxSkyBoxTech;
		D3DX11_TECHNIQUE_DESC techDesc;
		tech->GetDesc(&techDesc);

		for(UINT p=0; p<techDesc.Passes; ++p)
		{
			XMFLOAT3 pos = tmpCamera->GetPosition();
			XMMATRIX worldTrans = XMMatrixTranslation(pos.x, pos.y, pos.z);
			XMMATRIX wvp = worldTrans * tmpCamera->ViewProjection();
			Effects::fxSkyBox->SetWorldViewProjMatrix(wvp);
			Effects::fxSkyBox->SetCubeMap(m_cubeMapSRV);

			tech->GetPassByIndex(p)->Apply(0,m_deviceContext);
			m_deviceContext->DrawIndexed(m_skySphere.indices.size(),0,0);
			//Restore render states for other renderings
			m_deviceContext->RSSetState(0);
		}

		m_deviceContext->IASetInputLayout(InputLayouts::basic32);
		UINT stride2 = sizeof(Vertex::Basic32);
		UINT offset2 = 0;
		m_deviceContext->IASetVertexBuffers(0,1,&m_VBObjects,&stride2,&offset2);
		m_deviceContext->IASetIndexBuffer(m_IBObjects,DXGI_FORMAT_R32_UINT,0);

		ID3DX11EffectTechnique *mainTech1 = Effects::fxBasic->fxLight3TexTech;
		D3DX11_TECHNIQUE_DESC mainTechDesc1;
		mainTech1->GetDesc(&mainTechDesc1);

		for(UINT p=0; p<mainTechDesc1.Passes; ++p)
		{
			XMMATRIX world = XMLoadFloat4x4(&m_worldBox);
			XMMATRIX wvp = world * tmpCamera->ViewProjection();
			XMMATRIX invWorldTrans = XMLoadFloat4x4(&m_invWorldTransposeBox);
			Effects::fxBasic->SetWorldMatrix(world);
			Effects::fxBasic->SetWorldViewProjMatrix(wvp);
			Effects::fxBasic->SetWorldInvTransposeMatrix(invWorldTrans);
			Effects::fxBasic->SetTextureTransform(XMMatrixIdentity());
			Effects::fxBasic->SetMaterial(m_material);
			Effects::fxBasic->SetShaderResource(m_boxSRV);

			mainTech1->GetPassByIndex(p)->Apply(0,m_deviceContext);
			m_deviceContext->DrawIndexed(m_box.indices.size(),m_boxIStart,m_boxVStart);
			//Restore render states for other renderings
			m_deviceContext->RSSetState(0);
		}

	}
	//Generate mip maps for the dynamic cube map
	m_deviceContext->GenerateMips(m_dynamicSRV);

	//Now begin rendering the scenen to the back buffer, including the central sphere rendered using the newly generated cube map
	m_deviceContext->OMSetRenderTargets(1,&m_renderTargetView,m_depthStencilView);
	m_deviceContext->RSSetViewports(1,&m_viewport);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView,reinterpret_cast<const float*>(&Colors::Silver));
	m_deviceContext->ClearDepthStencilView(m_depthStencilView,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.f,0);
	
	//Three techniques: one for sphere, one for box, one for sky box
	ID3DX11EffectTechnique *mainTech = Effects::fxBasic->fxLight3ReflectionTech;
	ID3DX11EffectTechnique *mainTech2 = Effects::fxBasic->fxLight3TexTech;
	ID3DX11EffectTechnique *tech = Effects::fxSkyBox->fxSkyBoxTech;
	
	D3DX11_TECHNIQUE_DESC mainTechDesc,mainTechDesc2,techDesc;
	
	mainTech->GetDesc(&mainTechDesc);
	mainTech2->GetDesc(&mainTechDesc2);
	tech->GetDesc(&techDesc);
	
	//Begin rendering sphere
	m_deviceContext->IASetInputLayout(InputLayouts::basic32);
	UINT stride1 = sizeof(Vertex::Basic32);
	UINT offset1 = 0;
	m_deviceContext->IASetVertexBuffers(0,1,&m_VBObjects,&stride1,&offset1);
	m_deviceContext->IASetIndexBuffer(m_IBObjects,DXGI_FORMAT_R32_UINT,0);
	
	for(UINT i=0; i<mainTechDesc.Passes; ++i)
	{
		//Update per object shader variables
		XMMATRIX world = XMLoadFloat4x4(&m_worldSphere);
		XMMATRIX wvp = world * m_camera.ViewProjection();
		XMMATRIX invWorldTrans = XMLoadFloat4x4(&m_invWorldTranspose);
		Effects::fxBasic->SetWorldMatrix(world);
		Effects::fxBasic->SetWorldViewProjMatrix(wvp);
		Effects::fxBasic->SetWorldInvTransposeMatrix(invWorldTrans);
		Effects::fxBasic->SetCubeMap(m_dynamicSRV);
		Effects::fxBasic->SetMaterial(m_material);

		mainTech->GetPassByIndex(i)->Apply(0,m_deviceContext);
		m_deviceContext->DrawIndexed(m_sphere.indices.size(),m_sphereIStart,m_sphereVStart);
		//Restore render states for other renderings
		m_deviceContext->RSSetState(0);


	}
	for(UINT i=0; i<mainTechDesc2.Passes; ++i)
	{
		XMMATRIX world = XMLoadFloat4x4(&m_worldBox);
		XMMATRIX wvp = world * m_camera.ViewProjection();
		XMMATRIX invWorldTrans = XMLoadFloat4x4(&m_invWorldTransposeBox);
		Effects::fxBasic->SetWorldMatrix(world);
		Effects::fxBasic->SetWorldViewProjMatrix(wvp);
		Effects::fxBasic->SetWorldInvTransposeMatrix(invWorldTrans);
		Effects::fxBasic->SetTextureTransform(XMMatrixIdentity());
		Effects::fxBasic->SetMaterial(m_material);
		Effects::fxBasic->SetShaderResource(m_boxSRV);

		mainTech2->GetPassByIndex(i)->Apply(0,m_deviceContext);
		m_deviceContext->DrawIndexed(m_box.indices.size(),m_boxIStart,m_boxVStart);
		//Restore render states for other renderings
		m_deviceContext->RSSetState(0);
	}
	
	//Begin rendering sky box
	m_deviceContext->IASetInputLayout(InputLayouts::pos);
	UINT stride2 = sizeof(PosVertex);
	UINT offset2 = 0;
	m_deviceContext->IASetVertexBuffers(0,1,&m_VBSky,&stride2,&offset2);
	m_deviceContext->IASetIndexBuffer(m_IBSky,DXGI_FORMAT_R32_UINT,0);
	for(UINT i=0; i<techDesc.Passes; ++i)
	{
		//Update per obejct shader variables
		XMFLOAT3 eyePos = m_camera.GetPosition();
		XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
		XMMATRIX WVP = XMMatrixMultiply(T, m_camera.ViewProjection());
		Effects::fxSkyBox->SetWorldViewProjMatrix(WVP);
		Effects::fxSkyBox->SetCubeMap(m_cubeMapSRV);

		tech->GetPassByIndex(i)->Apply(0,m_deviceContext);
		m_deviceContext->DrawIndexed(m_skySphere.indices.size(),0,0);
		//Restore render states for other renderings
		m_deviceContext->RSSetState(0);
	}
	
	m_swapChain->Present(0,0);

	return true;
}

void DynamicCubeMapping::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastPos.x = x;
	m_lastPos.y = y;
	SetCapture(m_hWnd);
}

void DynamicCubeMapping::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void DynamicCubeMapping::OnMouseMove(WPARAM btnState, int x, int y)
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
	DynamicCubeMapping demo(hInstance);
	if(!demo.Init())
		return -1;

	return demo.Run(); 
}