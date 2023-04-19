#include "Effects.h"
#include <vector>
#include <fstream>

using namespace std;

bool Effect::Init(ID3D11Device *device,std::wstring fileName)
{
	vector<char> shader;
	if(!ReadBinaryFile(fileName,shader))
		return false;

	if(FAILED(D3DX11CreateEffectFromMemory(&shader[0],shader.size(),0,device,&fx)))
	{
		MessageBox(NULL,L"Create Effect failed!",L"Error",MB_OK);
		return false;
	}

	return true;
}

bool BasicColorEffect::Init(ID3D11Device *device,std::wstring fileName)
{
	if(!Effect::Init(device,fileName))
		return false;

	fxWorldViewProj = fx->GetVariableByName("g_worldViewProj")->AsMatrix();
	fxBasicColorTech = fx->GetTechniqueByName("BasicColor");

	return true;
}

bool BasicEffect::Init(ID3D11Device *device, std::wstring fileName)
{
	if(!Effect::Init(device,fileName))
		return false;

	fxWorldViewProj = fx->GetVariableByName("g_worldViewProj")->AsMatrix();
	fxWorld = fx->GetVariableByName("g_world")->AsMatrix();
	fxWorldInvTranspose = fx->GetVariableByName("g_worldInvTranspose")->AsMatrix();
	fxMaterial = fx->GetVariableByName("g_material");
	fxTexTrans = fx->GetVariableByName("g_texTrans")->AsMatrix();
	fxShadowTrans = fx->GetVariableByName("g_shadowTrans")->AsMatrix();
	fxSR = fx->GetVariableByName("g_tex")->AsShaderResource();
	fxCubeMap = fx->GetVariableByName("g_envMap")->AsShaderResource();
	fxShadowMap = fx->GetVariableByName("g_shadowMap")->AsShaderResource();

	fxDirLights = fx->GetVariableByName("g_lights");
	fxEyePos = fx->GetVariableByName("g_eyePos");
	fxFogStart = fx->GetVariableByName("g_fogStart")->AsScalar();
	fxFogRange = fx->GetVariableByName("g_fogRange")->AsScalar();
	fxFogColor = fx->GetVariableByName("g_fogColor")->AsVector();


	fxLight1Tech = fx->GetTechniqueByName("Light1");
	fxLight2Tech = fx->GetTechniqueByName("Light2");
	fxLight3Tech = fx->GetTechniqueByName("Light3");
	fxLight1TexTech = fx->GetTechniqueByName("Light1Tex");
	fxLight2TexTech = fx->GetTechniqueByName("Light2Tex");
	fxLight3TexTech = fx->GetTechniqueByName("Light3Tex");
	fxLight1TexClipTech = fx->GetTechniqueByName("Light1TexClip");
	fxLight2TexClipTech = fx->GetTechniqueByName("Light2TexClip");
	fxLight3TexClipTech = fx->GetTechniqueByName("Light3TexClip");
	fxLight1FogTech = fx->GetTechniqueByName("Light1Fog");
	fxLight2FogTech = fx->GetTechniqueByName("Light2Fog");
	fxLight3FogTech = fx->GetTechniqueByName("Light3Fog");
	fxLight1TexFogTech = fx->GetTechniqueByName("Light1TexFog");
	fxLight2TexFogTech = fx->GetTechniqueByName("Light2TexFog");
	fxLight3TexFogTech = fx->GetTechniqueByName("Light3TexFog");
	fxLight1TexClipFogTech = fx->GetTechniqueByName("Light1TexClipFog");
	fxLight2TexClipFogTech = fx->GetTechniqueByName("Light2TexClipFog");
	fxLight3TexClipFogTech = fx->GetTechniqueByName("Light3TexClipFog");
	fxLight1FogClipTech = fx->GetTechniqueByName("Light1FogClip");
	fxLight2FogClipTech = fx->GetTechniqueByName("Light2FogClip");
	fxLight3FogClipTech = fx->GetTechniqueByName("Light3FogClip");
	fxLight1TexFogClipTech = fx->GetTechniqueByName("Light1TexFogClip");
	fxLight2TexFogClipTech = fx->GetTechniqueByName("Light2TexFogClip");
	fxLight3TexFogClipTech = fx->GetTechniqueByName("Light3TexFogClip");
	fxLight1TexClipFogClipTech = fx->GetTechniqueByName("Light1TexClipFogClip");
	fxLight2TexClipFogClipTech = fx->GetTechniqueByName("Light2TexClipFogClip");
	fxLight3TexClipFogClipTech = fx->GetTechniqueByName("Light3TexClipFogClip");

	fxLight3ReflectionTech = fx->GetTechniqueByName("Light3Reflection");

	fxLight3TexShadowMappingTech = fx->GetTechniqueByName("Light3TexShadowMapping");
	fxLight3TexClipShadowMappingTech = fx->GetTechniqueByName("Light3TexClipShadowMapping");
	fxLight3TexClipSoftShadowMappingTech = fx->GetTechniqueByName("Light3TexClipSoftShadowMapping");

	return true;
}

bool ShadowMappingEffect::Init(ID3D11Device *device, std::wstring fileName)
{
	if(!Effect::Init(device,fileName))
		return false;

	fxLightViewProjection = fx->GetVariableByName("g_lightViewProj")->AsMatrix();
	fxTextureTransform = fx->GetVariableByName("g_texTrans")->AsMatrix();
	fxSRV = fx->GetVariableByName("g_texture")->AsShaderResource();
	fxShadowMapTech = fx->GetTechniqueByName("ShadowMap");
	fxShadowMapAlphaClipTech = fx->GetTechniqueByName("ShadowMapAlphaClip");

	return true;
}

bool SkyBoxEffect::Init(ID3D11Device *device, std::wstring fileName)
{
	if(!Effect::Init(device,fileName))
		return false;

	fxWorldViewProj = fx->GetVariableByName("g_worldViewProj")->AsMatrix();
	fxCubeMap = fx->GetVariableByName("g_cubeMap")->AsShaderResource();
	fxSkyBoxTech = fx->GetTechniqueByName("SkyBoxTech");

	return true;
}

BasicEffect* Effects::fxBasic(NULL);
ShadowMappingEffect* Effects::fxShadowMapping(NULL);
SkyBoxEffect* Effects::fxSkyBox(NULL);
bool Effects::InitAll(ID3D11Device *device)
{
	if(!fxBasic)
	{
		fxBasic = new BasicEffect;
		if(!fxBasic->Init(device,L"FX/Basic.fxo"))
			return false;
	}
	if(!fxShadowMapping)
	{
		fxShadowMapping = new ShadowMappingEffect;
		if(!fxShadowMapping->Init(device,L"FX/BuildShadowMap.fxo"))
			return false;
	}
	if(!fxSkyBox)
	{
		fxSkyBox = new SkyBoxEffect;
		if(!fxSkyBox->Init(device,L"FX/SkyBox.fxo"))
			return false;
	}

	return true;
}

void Effects::ReleaseAll()
{
	SafeDelete(fxBasic);
	SafeDelete(fxShadowMapping);
	SafeDelete(fxSkyBox);
}