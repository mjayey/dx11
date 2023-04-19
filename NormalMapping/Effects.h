#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#include <Windows.h>
#include <xnamath.h>
#include <D3D11.h>
#include <d3dx11effect.h>
#include <D3DX11async.h>
#include <AppUtil.h>
#include <Lights.h>
#include <string>

//Effect base class
class Effect
{
public:
	Effect():fx(NULL){	}

	virtual ~Effect()
	{
		SafeRelease(fx);
	}

	//Initialize effects using 'device' and fx file 'fileName'
	virtual bool Init(ID3D11Device *device,std::wstring fileName);

	//Main effect interface
	ID3DX11Effect	*fx;

private:
	//No copy
	Effect(const Effect&);
	Effect& operator = (const Effect&);
};

//Derived class: Basic color effect
class BasicColorEffect: public Effect
{
public:
	BasicColorEffect():fxWorldViewProj(NULL),fxBasicColorTech(NULL){	}
	~BasicColorEffect(){	}

	bool Init(ID3D11Device *device,std::wstring fileName);

	void SetWorldViewProjMatrix(XMFLOAT4X4 worldViewProj)
	{
		fxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
	}

	ID3DX11EffectMatrixVariable	*fxWorldViewProj;
	ID3DX11EffectTechnique		*fxBasicColorTech;
};

//Basic effect, used for most cases.
class BasicEffect: public Effect
{
public:
	BasicEffect():fxWorldViewProj(NULL),
		fxWorld(NULL),
		fxWorldInvTranspose(NULL),
		fxMaterial(NULL),
		fxTexTrans(NULL),
		fxTexOffsetScale(NULL),
		fxDirLights(NULL),
		fxEyePos(NULL),
		fxHeightScale(NULL),
		fxFogColor(NULL),
		fxFogStart(NULL),
		fxFogRange(NULL),
		fxSR(NULL),
		fxLight1Tech(NULL),
		fxLight2Tech(NULL),
		fxLight3Tech(NULL),
		fxLight1NormalMappingTech(NULL),
		fxLight2NormalMappingTech(NULL),
		fxLight3NormalMappingTech(NULL),
		fxLight1NormalParallaxMappingTech(NULL),
		fxLight2NormalParallaxMappingTech(NULL),
		fxLight3NormalParallaxMappingTech(NULL),
		fxLight1TexTech(NULL),
		fxLight2TexTech(NULL),
		fxLight3TexTech(NULL),
		fxLight1TexClipTech(NULL),
		fxLight2TexClipTech(NULL),
		fxLight3TexClipTech(NULL),
		fxLight1TexNormalMappingTech(NULL),
		fxLight2TexNormalMappingTech(NULL),
		fxLight3TexNormalMappingTech(NULL),
		fxLight1TexNormalParallaxMappingTech(NULL),
		fxLight2TexNormalParallaxMappingTech(NULL),
		fxLight3TexNormalParallaxMappingTech(NULL),
		fxLight1TexShadowMappingTech(NULL),
		fxLight2TexShadowMappingTech(NULL),
		fxLight3TexShadowMappingTech(NULL),
		fxLight1TexPCFShadowMappingTech(NULL),
		fxLight2TexPCFShadowMappingTech(NULL),
		fxLight3TexPCFShadowMappingTech(NULL),
		fxLight1TexReflectionTech(NULL),
		fxLight2TexReflectionTech(NULL),
		fxLight3TexReflectionTech(NULL),
		fxLight1TexFogTech(NULL),
		fxLight2TexFogTech(NULL),
		fxLight3TexFogTech(NULL),
		fxLight1TexClipNormalMappingTech(NULL),
		fxLight2TexClipNormalMappingTech(NULL),
		fxLight3TexClipNormalMappingTech(NULL)
	{ }
	
	bool Init(ID3D11Device *device, std::wstring fileName);

	void SetWorldViewProjMatrix(CXMMATRIX M)				{ fxWorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M));	}
	void SetWorldMatrix(CXMMATRIX M)						{ fxWorld->SetMatrix(reinterpret_cast<const float*>(&M));	}
	void SetWorldInvTransposeMatrix(CXMMATRIX M)			{ fxWorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M));	}
	void SetMaterial(Lights::Material material)				{ fxMaterial->SetRawValue(&material,0,sizeof(material));	}
	void SetTextureTransform(CXMMATRIX M)					{ fxTexTrans->SetMatrix(reinterpret_cast<const float*>(&M));	}
	void SetShadowTransform(CXMMATRIX M)					{ fxShadowTrans->SetMatrix(reinterpret_cast<const float*>(&M));	}
	void SetTextureOffsetScale(XMFLOAT2 scale)				{ fxTexOffsetScale->SetRawValue(&scale,0,sizeof(scale));	}
	void SetHeightScale(float scale)						{ fxHeightScale->SetFloat(scale);	}
	void SetLights(Lights::DirLight *lights)				{ fxDirLights->SetRawValue(lights,0,3*sizeof(Lights::DirLight));	}
	void SetEyePos(XMFLOAT3 eyePos)							{ fxEyePos->SetRawValue(&eyePos,0,sizeof(eyePos));	}
	void SetShaderResource(ID3D11ShaderResourceView *srv)	{ fxSR->SetResource(srv);	}
	void SetNormalMap(ID3D11ShaderResourceView *normalMap)	{ fxNormalMap->SetResource(normalMap);	}
	void SetShadowMap(ID3D11ShaderResourceView *shadowMap)	{ fxShadowMap->SetResource(shadowMap);	}
	void SetCubeMap(ID3D11ShaderResourceView *cubeMap)		{ fxCubeMap->SetResource(cubeMap);	}
	
	void SetFogStart(float fogStart)		{ fxFogStart->SetFloat(fogStart);	}
	void SetFogRange(float fogRange)		{ fxFogRange->SetFloat(fogRange);	}
	void SetFogColor(FXMVECTOR fogColor)	{ fxFogColor->SetFloatVector(reinterpret_cast<const float*>(&fogColor));	}

	//Per object vars
	ID3DX11EffectMatrixVariable	*fxWorldViewProj;
	ID3DX11EffectMatrixVariable	*fxWorld;
	ID3DX11EffectMatrixVariable	*fxWorldInvTranspose;
	ID3DX11EffectVariable		*fxMaterial;
	ID3DX11EffectMatrixVariable	*fxTexTrans;
	ID3DX11EffectMatrixVariable *fxShadowTrans;
	ID3DX11EffectScalarVariable		*fxHeightScale;
	ID3DX11EffectVariable			*fxTexOffsetScale;
	
	//Texture
	ID3DX11EffectShaderResourceVariable	*fxSR;
	//Cube Map
	ID3DX11EffectShaderResourceVariable *fxCubeMap;
	//Normal map
	ID3DX11EffectShaderResourceVariable *fxNormalMap;
	//Shadow map
	ID3DX11EffectShaderResourceVariable *fxShadowMap;

	//Per frame vars
	ID3DX11EffectVariable			*fxDirLights;
	ID3DX11EffectVariable			*fxEyePos;
	ID3DX11EffectScalarVariable		*fxFogStart;
	ID3DX11EffectScalarVariable		*fxFogRange;
	ID3DX11EffectVectorVariable		*fxFogColor;

	//Techniques
	//Lighting
	ID3DX11EffectTechnique		*fxLight1Tech;
	ID3DX11EffectTechnique		*fxLight2Tech;
	ID3DX11EffectTechnique		*fxLight3Tech;
	//Lighting + Normal mapping
	ID3DX11EffectTechnique		*fxLight1NormalMappingTech;
	ID3DX11EffectTechnique		*fxLight2NormalMappingTech;
	ID3DX11EffectTechnique		*fxLight3NormalMappingTech;
	//Lighting + Normal mapping + parallax mapping
	ID3DX11EffectTechnique		*fxLight1NormalParallaxMappingTech;
	ID3DX11EffectTechnique		*fxLight2NormalParallaxMappingTech;
	ID3DX11EffectTechnique		*fxLight3NormalParallaxMappingTech;
	//Lighting + Texture
	ID3DX11EffectTechnique		*fxLight1TexTech;
	ID3DX11EffectTechnique		*fxLight2TexTech;
	ID3DX11EffectTechnique		*fxLight3TexTech;

	//Lighting + Texutre + Alpha clipping
	ID3DX11EffectTechnique		*fxLight1TexClipTech;
	ID3DX11EffectTechnique		*fxLight2TexClipTech;
	ID3DX11EffectTechnique		*fxLight3TexClipTech;

	//Lighting + Texture + Normal Mapping
	ID3DX11EffectTechnique		*fxLight1TexNormalMappingTech;
	ID3DX11EffectTechnique		*fxLight2TexNormalMappingTech;
	ID3DX11EffectTechnique		*fxLight3TexNormalMappingTech;

	//Lighting + Texture + Normal Mapping + Parallax Mapping
	ID3DX11EffectTechnique		*fxLight1TexNormalParallaxMappingTech;
	ID3DX11EffectTechnique		*fxLight2TexNormalParallaxMappingTech;
	ID3DX11EffectTechnique		*fxLight3TexNormalParallaxMappingTech;
	
	//Lighting + Texture + Shadow Mapping
	ID3DX11EffectTechnique		*fxLight1TexShadowMappingTech;
	ID3DX11EffectTechnique		*fxLight2TexShadowMappingTech;
	ID3DX11EffectTechnique		*fxLight3TexShadowMappingTech;
	
	//Lighting + Texture + PCF soft Shadow Mapping
	ID3DX11EffectTechnique		*fxLight1TexPCFShadowMappingTech;
	ID3DX11EffectTechnique		*fxLight2TexPCFShadowMappingTech;
	ID3DX11EffectTechnique		*fxLight3TexPCFShadowMappingTech;
	
	//Lighting + Texture + cube mapping Reflection
	ID3DX11EffectTechnique		*fxLight1TexReflectionTech;
	ID3DX11EffectTechnique		*fxLight2TexReflectionTech;
	ID3DX11EffectTechnique		*fxLight3TexReflectionTech;
	
	//Lighting + Texture + Fog effect
	ID3DX11EffectTechnique		*fxLight1TexFogTech;
	ID3DX11EffectTechnique		*fxLight2TexFogTech;
	ID3DX11EffectTechnique		*fxLight3TexFogTech;
	
	//Lighting + Texture + Alpha clipping + Normal Mapping
	ID3DX11EffectTechnique		*fxLight1TexClipNormalMappingTech;
	ID3DX11EffectTechnique		*fxLight2TexClipNormalMappingTech;
	ID3DX11EffectTechnique		*fxLight3TexClipNormalMappingTech;
};

//Effect used for shadow mapping
class ShadowMappingEffect: public Effect
{
public:
	bool Init(ID3D11Device *device, std::wstring fileName);

	void SetLightViewProjectionMatrix(CXMMATRIX lvp)	{ fxLightViewProjection->SetMatrix(reinterpret_cast<const float*>(&lvp)); }
	void SetTextureTransformation(CXMMATRIX texTrans)	{ fxTextureTransform->SetMatrix(reinterpret_cast<const float*>(&texTrans)); }
	void SetSRV(ID3D11ShaderResourceView *srv)			{ fxSRV->SetResource(srv); }

	ID3DX11EffectMatrixVariable				*fxLightViewProjection;
	ID3DX11EffectMatrixVariable				*fxTextureTransform;
	ID3DX11EffectShaderResourceVariable		*fxSRV;

	ID3DX11EffectTechnique	*fxShadowMapTech;
	ID3DX11EffectTechnique	*fxShadowMapAlphaClipTech;
};

class SkyBoxEffect: public Effect
{
public:
	bool Init(ID3D11Device *device, std::wstring fileName);

	void SetWorldViewProjMatrix(CXMMATRIX wvp) { fxWorldViewProj->SetMatrix(reinterpret_cast<const float*>(&wvp));	}
	void SetCubeMap(ID3D11ShaderResourceView *cubeMap) { fxCubeMap->SetResource(cubeMap);	}

	ID3DX11EffectMatrixVariable				*fxWorldViewProj;
	ID3DX11EffectShaderResourceVariable		*fxCubeMap;
	ID3DX11EffectTechnique					*fxSkyBoxTech;
};

class Effects
{
public:
	static bool InitAll(ID3D11Device *device);
	static void ReleaseAll();

	static BasicEffect			*fxBasic;
	static ShadowMappingEffect	*fxShadowMapping;
	static SkyBoxEffect			*fxSkyBox;
};
#endif	//_EFFECTS_H_