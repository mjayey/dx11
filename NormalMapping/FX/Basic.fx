//Base effect file with many techniques

#include "Light.fx"

//Variables for per object rendering
cbuffer	PerObject
{
	float4x4	g_worldViewProj;		//World * View * Projection
	float4x4	g_world;				//World matrix
	float4x4	g_worldInvTranspose;	//Inverse transpose of world matrix(for vertex normal transformation)
	Material	g_material;				//Material
	float4x4	g_texTrans;				//Texture transformation
	float4x4	g_shadowTrans;			//Shadow transformation
};

//Variables for per frame rendering
cbuffer	PerFrame
{
	DirLight	g_lights[3];			//3 directional lights

	float3		g_eyePos;				//view point

	float4		g_fogColor;				//fog color
	float		g_fogStart;				//start distance of fog
	float		g_fogRange;				//range of fog
};

//Texture used for renderig
Texture2D	g_tex;				//2d texture
TextureCube	g_envMap;			//Environment map
Texture2D	g_shadowMap;		//Shadow map

SamplerState samplerTex
{
	Filter = MIN_MAG_MIP_LINEAR;		//Linear filtering for minification, magnification and mipmap choosing
	AddressU = Wrap;
	AddressV = Wrap;
};

//For shadow mapping
SamplerComparisonState samShadow
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = Border;
	AddressV = Border;
	AddressW = Border;
	BorderColor = float4(0.f,0.f,0.f,0.f);

	ComparisonFunc = LESS;
};

struct VertexIn
{
	float3	pos		: POSITION;		//Vertex position
	float3	normal	: NORMAL;		//Vertex normal
	float2	tex		: TEXCOORD;		//Texture coordination
};

struct VertexOut
{
	float3	posTrans   : POSITION;		//position after world transformation
	float4	posH	   : SV_POSITION;	//position after projection
	float3	normal	   : NORMAL;		//vertex normal after world transformation
	float2	tex		   : TEXCOORD0;		//Texture coordination after transformation
	float4	shadowPosH : TEXCOORD1;
};

//Vertex Shader
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.posTrans = mul(float4(vin.pos,1.f),g_world).xyz;
	vout.posH = mul(float4(vin.pos,1.f),g_worldViewProj);
	vout.normal = mul(vin.normal,(float3x3)g_worldInvTranspose);
	vout.tex = mul(float4(vin.tex,0.f,1.f),g_texTrans).xy;
	vout.shadowPosH = mul(float4(vin.pos,1.f),g_shadowTrans);

	return vout;
}

//Pixel Shader
float4 PS(VertexOut pin,
		uniform int numLights, 
		uniform bool useTexture, uniform bool AlphaClipEnable, 
		uniform bool useFog, uniform bool fogClipEnable,
		uniform bool useReflection,
		uniform bool shadowMappingEnable, uniform bool pcfShadow): SV_TARGET
{
	float3 toEye = g_eyePos - pin.posTrans;
	float	dist = length(toEye);					//Distance to eye
	toEye /= dist;

	float3 normal = normalize(pin.normal);

	float4 texColor = float4(1.f,1.f,1.f,1.f);			//Default color
	
	if(useTexture)										//If use texture
	{
		texColor = g_tex.Sample(samplerTex,pin.tex);
		
		if(AlphaClipEnable)								//If use texture alpha clipping
		{
			clip(texColor.a-0.1f);
		}
	}

	float4 litColor = texColor;
	
	if(numLights > 0)									//Lighting calculation
	{
		float4 A = float4(0.f,0.f,0.f,0.f);
		float4 D = float4(0.f,0.f,0.f,0.f);
		float4 S = float4(0.f,0.f,0.f,0.f);

		float3 shadowFactor = {1.f,1.f,1.f};
		if(shadowMappingEnable)
		{
			if(!pcfShadow)
				shadowFactor[0] = CalculateShadowFactor(samShadow,g_shadowMap,pin.shadowPosH);
			else
				shadowFactor[0] = CalculateShadowFactor3x3(samShadow,g_shadowMap,pin.shadowPosH);
		}

		[unroll]
		for(int i=0; i<numLights; ++i)					//Calculate lights one by one, and add the results together
		{
			float4 ambient, diff, spec;
			ComputeDirLight(g_material,g_lights[i],normal,toEye, ambient,diff,spec);
			A += ambient;
			D += shadowFactor[i] * diff;
			S += shadowFactor[i] * spec;
		}
		
		litColor = texColor * (A + D) + S;				//Result color: textureColor * (Ambient + Diffuse) + Specular
	}

	if(useFog)											//If use fog effect
	{
		if(fogClipEnable)								//Fog clipping, clip fragments that cannot be seen
		{
			clip(g_fogStart+g_fogRange-dist);
		}

		float fogFactor = saturate((dist - g_fogStart)/g_fogRange);
		litColor = lerp(litColor,g_fogColor,fogFactor);				//Final color with fog
	}
	
	//Use cube mapping for reflection effect
	if(useReflection)
	{
		float3 ref = reflect(-toEye,normal);
		float4 refColor = g_envMap.Sample(samplerTex,ref);
		//litColor += g_material.reflection * refColor;
		litColor = lerp(litColor,refColor,g_material.reflection);
	}
	
	litColor.a = texColor.a * g_material.diffuse.a;		//Alpha element

	return litColor;
}

//Different techniques
technique11 Light1
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(1,false,false,false,false,false,false,false)) );
	}
}

technique11 Light2
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(2,false,false,false,false,false,false,false)) );
	}
}

technique11 Light3
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,false,false,false,false,false,false,false)) );
	}
}

technique11 Light1Tex
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(1,true,false,false,false,false,false,false)) );
	}
}

technique11 Light2Tex
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(2,true,false,false,false,false,false,false)) );
	}
}

technique11 Light3Tex
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,true,false,false,false,false,false,false)) );
	}
}

technique11 Light1TexClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(1,true,true,false,false,false,false,false)) );
	}
}

technique11 Light2TexClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(2,true,true,false,false,false,false,false)) );
	}
}

technique11 Light3TexClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,true,true,false,false,false,false,false)) );
	}
}

technique11 Light1Fog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(1,false,false,true,false,false,false,false)));
	}
}

technique11 Light2Fog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(2,false,false,true,false,false,false,false)));
	}
}

technique11 Light3Fog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,false,false,true,false,false,false,false)));
	}
}

technique11 Light1TexFog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(1,true,false,true,false,false,false,false)));
	}
}

technique11 Light2TexFog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(2,true,false,true,false,false,false,false)));
	}
}

technique11 Light3TexFog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,true,false,true,false,false,false,false)));
	}
}

technique11 Light1TexClipFog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(1,true,true,true,false,false,false,false)));
	}
}

technique11 Light2TexClipFog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(2,true,true,true,false,false,false,false)));
	}
}

technique11 Light3TexClipFog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,true,true,true,false,false,false,false)));
	}
}

technique11 Light1FogClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(1,false,false,true,true,false,false,false)));
	}
}

technique11 Light2FogClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(2,false,false,true,true,false,false,false)));
	}
}

technique11 Light3FogClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,false,false,true,true,false,false,false)));
	}
}

technique11 Light1TexFogClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(1,true,false,true,true,false,false,false)));
	}
}

technique11 Light2TexFogClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(2,true,false,true,true,false,false,false)));
	}
}

technique11 Light3TexFogClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,true,false,true,true,false,false,false)));
	}
}

technique11 Light1TexClipFogClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(1,true,true,true,true,false,false,false)));
	}
}

technique11 Light2TexClipFogClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(2,true,true,true,true,false,false,false)));
	}
}

technique11 Light3TexClipFogClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,true,true,true,true,false,false,false)));
	}
}


technique11 Light3Reflection
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,false,false,false,false,true,false,false)));
	}
}

technique11 Light3TexShadowMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,true,false,false,false,false,true,false)));
	}
}

technique11 Light3TexClipShadowMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,true,true,false,false,false,true,false)));
	}
}

technique11 Light3TexClipSoftShadowMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0,VS()) );
		SetPixelShader( CompileShader(ps_4_0,PS(3,true,true,false,false,false,true,true)));
	}
}
