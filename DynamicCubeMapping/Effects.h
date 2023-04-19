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
		fxDirLights(NULL),
		fxEyePos(NULL),
		fxSR(NULL),
		fxLight1Tech(NULL),
		fxLight2Tech(NULL),
		fxLight3Tech(NULL),
		fxLight1TexTech(NULL),
		fxLight2TexTech(NULL),
		fxLight3TexTech(NULL),
		fxLight1TexClipTech(NULL),
		fxLight2TexClipTech(NULL),
		fxLight3TexClipTech(NULL),
		fxLight1FogTech(NULL),
		fxLight2FogTech(NULL),
		fxLight3FogTech(NULL),
		fxLight1TexFogTech(NULL),
		fxLight2TexFogTech(NULL),
		fxLight3TexFogTech(NULL),
		fxLight1TexClipFogTech(NULL),
		fxLight2TexClipFogTech(NULL),
		fxLight3TexClipFogTech(NULL),
		fxLight3TexShadowMappingTech(NULL),
		fxLight3TexClipShadowMappingTech(NULL)
	{ }
	
	bool Init(ID3D11Device *device, std::wstring fileName);

	void SetWorldViewProjMatrix(CXMMATRIX M)				{ fxWorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M));	}
	void SetWorldMatrix(CXMMATRIX M)						{ fxWorld->SetMatrix(reinterpret_cast<const float*>(&M));	}
	void SetWorldInvTransposeMatrix(CXMMATRIX M)			{ fxWorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M));	}
	void SetMaterial(Lights::Material material)				{ fxMaterial->SetRawValue(&material,0,sizeof(material));	}
	void SetTextureTransform(CXMMATRIX M)					{ fxTexTrans->SetMatrix(reinterpret_cast<const float*>(&M));	}
	void SetShadowTransform(CXMMATRIX M)					{ fxShadowTrans->SetMatrix(reinterpret_cast<const float*>(&M));	}
	void SetLights(Lights::DirLight *lights)				{ fxDirLights->SetRawValue(lights,0,3*sizeof(Lights::DirLight));	}
	void SetEyePos(XMFLOAT3 eyePos)							{ fxEyePos->SetRawValue(&eyePos,0,sizeof(eyePos));	}
	void SetShaderResource(ID3D11ShaderResourceView *srv)	{ fxSR->SetResource(srv);	}
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
	
	//Texture
	ID3DX11EffectShaderResourceVariable	*fxSR;
	//Cube Map
	ID3DX11EffectShaderResourceVariable *fxCubeMap;
	//Shadow map
	ID3DX11EffectShaderResourceVariable *fxShadowMap;

	//Per frame vars
	ID3DX11EffectVariable			*fxDirLights;
	ID3DX11EffectVariable			*fxEyePos;
	ID3DX11EffectScalarVariable		*fxFogStart;
	ID3DX11EffectScalarVariable		*fxFogRange;
	ID3DX11EffectVectorVariable		*fxFogColor;

	//all techniques
	ID3DX11EffectTechnique		*fxLight1Tech;
	ID3DX11EffectTechnique		*fxLight2Tech;
	ID3DX11EffectTechnique		*fxLight3Tech;

	ID3DX11EffectTechnique		*fxLight1TexTech;
	ID3DX11EffectTechnique		*fxLight2TexTech;
	ID3DX11EffectTechnique		*fxLight3TexTech;

	ID3DX11EffectTechnique		*fxLight1TexClipTech;
	ID3DX11EffectTechnique		*fxLight2TexClipTech;
	ID3DX11EffectTechnique		*fxLight3TexClipTech;

	ID3DX11EffectTechnique		*fxLight1FogTech;
	ID3DX11EffectTechnique		*fxLight2FogTech;
	ID3DX11EffectTechnique		*fxLight3FogTech;

	ID3DX11EffectTechnique		*fxLight1TexFogTech;
	ID3DX11EffectTechnique		*fxLight2TexFogTech;
	ID3DX11EffectTechnique		*fxLight3TexFogTech;

	ID3DX11EffectTechnique		*fxLight1TexClipFogTech;
	ID3DX11EffectTechnique		*fxLight2TexClipFogTech;
	ID3DX11EffectTechnique		*fxLight3TexClipFogTech;

	ID3DX11EffectTechnique		*fxLight1FogClipTech;
	ID3DX11EffectTechnique		*fxLight2FogClipTech;
	ID3DX11EffectTechnique		*fxLight3FogClipTech;

	ID3DX11EffectTechnique		*fxLight1TexFogClipTech;
	ID3DX11EffectTechnique		*fxLight2TexFogClipTech;
	ID3DX11EffectTechnique		*fxLight3TexFogClipTech;

	ID3DX11EffectTechnique		*fxLight1TexClipFogClipTech;
	ID3DX11EffectTechnique		*fxLight2TexClipFogClipTech;
	ID3DX11EffectTechnique		*fxLight3TexClipFogClipTech;

	ID3DX11EffectTechnique		*fxLight3ReflectionTech;

	ID3DX11EffectTechnique		*fxLight3TexShadowMappingTech;
	ID3DX11EffectTechnique		*fxLight3TexClipShadowMappingTech;
	ID3DX11EffectTechnique		*fxLight3TexClipSoftShadowMappingTech;
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