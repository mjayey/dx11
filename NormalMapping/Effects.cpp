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
	fxTexOffsetScale = fx->GetVariableByName("g_texOffsetScale");
	fxHeightScale = fx->GetVariableByName("g_heightScale")->AsScalar();
	fxSR = fx->GetVariableByName("g_texture")->AsShaderResource();
	fxCubeMap = fx->GetVariableByName("g_cubeMap")->AsShaderResource();
	fxNormalMap = fx->GetVariableByName("g_normalMap")->AsShaderResource();
	fxShadowMap = fx->GetVariableByName("g_shadowMap")->AsShaderResource();

	fxDirLights = fx->GetVariableByName("g_dirLights");
	fxEyePos = fx->GetVariableByName("g_eyePos");
	fxFogStart = fx->GetVariableByName("g_fogStart")->AsScalar();
	fxFogRange = fx->GetVariableByName("g_fogRange")->AsScalar();
	fxFogColor = fx->GetVariableByName("g_fogColor")->AsVector();

	fxLight1Tech = fx->GetTechniqueByName("Light1");
	fxLight2Tech = fx->GetTechniqueByName("Light2");
	fxLight3Tech = fx->GetTechniqueByName("Light3");
	fxLight1NormalMappingTech = fx->GetTechniqueByName("Light1NormalMapping");
	fxLight2NormalMappingTech = fx->GetTechniqueByName("Light2NormalMapping");
	fxLight3NormalMappingTech = fx->GetTechniqueByName("Light3NormalMapping");
	fxLight1NormalParallaxMappingTech = fx->GetTechniqueByName("Light1NormalParallaxMapping");
	fxLight2NormalParallaxMappingTech = fx->GetTechniqueByName("Light2NormalParallaxMapping");
	fxLight3NormalParallaxMappingTech = fx->GetTechniqueByName("Light3NormalParallaxMapping");
	fxLight1TexTech = fx->GetTechniqueByName("Light1Texture");
	fxLight2TexTech = fx->GetTechniqueByName("Light2Texture");
	fxLight3TexTech = fx->GetTechniqueByName("Light3Texture");
	fxLight1TexClipTech = fx->GetTechniqueByName("Light1TexAlphaClip");
	fxLight2TexClipTech = fx->GetTechniqueByName("Light2TexAlphaClip");
	fxLight3TexClipTech = fx->GetTechniqueByName("Light3TexAlphaClip");
	fxLight1TexNormalMappingTech = fx->GetTechniqueByName("Light1TexNormalMapping");
	fxLight2TexNormalMappingTech = fx->GetTechniqueByName("Light2TexNormalMapping");
	fxLight3TexNormalMappingTech = fx->GetTechniqueByName("Light3TexNormalMapping");
	fxLight1TexNormalParallaxMappingTech = fx->GetTechniqueByName("Light1TexNormalParallaxMapping");
	fxLight2TexNormalParallaxMappingTech = fx->GetTechniqueByName("Light2TexNormalParallaxMapping");
	fxLight3TexNormalParallaxMappingTech = fx->GetTechniqueByName("Light3TexNormalParallaxMapping");
	fxLight1TexShadowMappingTech = fx->GetTechniqueByName("Light1TexShadowMapping");
	fxLight2TexShadowMappingTech = fx->GetTechniqueByName("Light2TexShadowMapping");
	fxLight3TexShadowMappingTech = fx->GetTechniqueByName("Light3TexShadowMapping");
	fxLight1TexPCFShadowMappingTech = fx->GetTechniqueByName("Light1TexPCFShadowMapping");
	fxLight2TexPCFShadowMappingTech = fx->GetTechniqueByName("Light2TexPCFShadowMapping");
	fxLight3TexPCFShadowMappingTech = fx->GetTechniqueByName("Light3TexPCFShadowMapping");
	fxLight1TexReflectionTech = fx->GetTechniqueByName("Light1TexRefelction");
	fxLight2TexReflectionTech = fx->GetTechniqueByName("Light2TexRefelction");
	fxLight3TexReflectionTech = fx->GetTechniqueByName("Light3TexRefelction");
	fxLight1TexFogTech = fx->GetTechniqueByName("Light1TexFog");
	fxLight2TexFogTech = fx->GetTechniqueByName("Light2TexFog");
	fxLight3TexFogTech = fx->GetTechniqueByName("Light3TexFog");
	fxLight1TexClipNormalMappingTech = fx->GetTechniqueByName("Light1TexAlphaClipNormaMapping");
	fxLight2TexClipNormalMappingTech = fx->GetTechniqueByName("Light2TexAlphaClipNormaMapping");
	fxLight3TexClipNormalMappingTech = fx->GetTechniqueByName("Light3TexAlphaClipNormaMapping");
	
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
		if(!fxBasic->Init(device,L"FX/NormalMapping.fxo"))
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