#include "Light.fx"

//Constant variables for each object
cbuffer PerObject
{
	float4x4	g_world;				//World matrix
	float4x4	g_worldViewProj;		//World view projection matrix
	float4x4	g_worldInvTranspose;	//Inverse transpose of world matrix
	float4x4	g_texTrans;				//Texture transformation matrix
	float4x4	g_shadowTrans;			//Shadow projection mapping matrix
	Material	g_material;				//Material

	float		g_heightScale;
	float2		g_texOffsetScale;
};

//Constant variables for each frame
cbuffer PerFrame
{
	DirLight	g_dirLights[3];			//Three directional lights
	float3		g_eyePos;				//Eye position

	float4		g_fogColor;				//Full fog color
	float		g_fogStart;				//Start distance of fog
	float		g_fogRange;				//The range of fog
};

//Input vertex
struct VertexIn
{
	float3	pos		: POSITION;			//Local space position
	float3	normal	: NORMAL;			//normal
	float3	tangent : TANGENT;			//tangent
	float2	tex		: TEXCOORD;			//texture coordiation (u,v)
};

//Output vertex of vertex shader
struct VertexOut
{
	float3	posL		: POSITION;			//World space position
	float4	posH		: SV_POSITION;		//Projection space position
	float3	normal		: NORMAL;			//World space normal
	float3	tangent		: TANGENT;			//World space tangent
	float2	tex			: TEXCOORD0;		//Texture coordiation
	float4	shadowTex	: TEXCOORD1;		//Shadow map coordiation
};

Texture2D		g_texture;			//Normal image texture
TextureCube		g_cubeMap;			//Environment map used for reflection
Texture2D		g_normalMap;		//Normal map
Texture2D		g_shadowMap;		//Shadow map

//For 2d texture
SamplerState samplerTex
{
	Filter = MIN_MAG_MIP_LINEAR;
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

//Vertex shader
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.posL = mul(float4(vin.pos,1.f), g_world).xyz;
	vout.posH = mul(float4(vin.pos,1.f),g_worldViewProj);
	vout.normal = mul(vin.normal, (float3x3)g_worldInvTranspose);
	vout.tangent = mul(vin.tangent,(float3x3)g_world);
	vout.tex = mul(float4(vin.tex,0.f,1.f),g_texTrans).xy;
	vout.shadowTex = mul(float4(vin.pos,1.f),g_shadowTrans);

	return vout;
}

//Pixel shader
float4 PS(VertexOut pin,
			uniform int numLights,				//number of lights used
			uniform bool useTexture,			//if use texture
			uniform bool alphaClipEnable,		//if enable alpha clipping
			uniform bool useNormalMap,			//if enable normal mapping
			uniform bool useParallaxMapping,	//if enable parallax mapping
			uniform bool useShadowMap,			//if enable shadow mapping
			uniform bool pcfShadowEnable,		//if enable pcf soft shadow mapping
			uniform bool useReflection,			//if enable cube mapping for reflection
			uniform bool fogEnable				//if enable fog effect
		): SV_TARGET
{
	//To eye vector
	float3 toEye = g_eyePos - pin.posL;
	float dist = length(toEye);
	toEye /= dist;
	
	//Default Normal we will use: Interpolated vertex normal
	float3 normal = normalize(pin.normal);
	
	//Get TBN space
	float3 N = normal;
	float3 T = normalize(pin.tangent - N * pin.tangent * N);
	float3 B = cross(N, T);

	//Parallax mapping
	if(useParallaxMapping)
	{
		float height = g_normalMap.Sample(samplerTex,pin.tex).a;		//Get height value from the 'alpha' component of the normal map
		height = (height - 1.f) * g_heightScale;						//[0,1] to [-g_heightScale,0]
		float3x3 W2T = transpose(float3x3(T,B,N));
		float3 toEyeTangent = mul(toEye, W2T);							//Transform to tangent space
		float2 offset = toEyeTangent.xy * height;						//Calculate offset according to height
		offset *= g_texOffsetScale;
		pin.tex += offset;												//Offset it
	}

	//Normal Mapping
	if(useNormalMap)
	{
		float3x3 T2W = float3x3(T, B, N);

		normal = g_normalMap.Sample(samplerTex, pin.tex).rgb;		//Get normal from the normal map
		normal = 2 * normal - 1;									//From [0,1] to [-1,1]
		normal = normalize(mul(normal, T2W));						//Transform the normal to world space
	}

	float4 texColor = float4(1.f,1.f,1.f,1.f);
	if(useTexture)
	{
		texColor = g_texture.Sample(samplerTex,pin.tex);
	}
	//Default color : Texture color only
	float4 color = texColor;

	//Lighting
	if(numLights > 0)
	{
		float4 ambient = float4(0.f,0.f,0.f,0.f);
		float4 diffuse = float4(0.f,0.f,0.f,0.f);
		float4 specular = float4(0.f,0.f,0.f,0.f);

		//For shadow calculating, 0.f means completely in shadow
		//Default: no shadow (1.f)
		float3 shadowFactor = {1.f, 1.f, 1.f};
		//Shadow mapping
		if(useShadowMap)
		{
			//PCF soft shadow
			if(pcfShadowEnable)
				shadowFactor[0] = CalculateShadowFactor3x3(samShadow,g_shadowMap,pin.shadowTex);
			else
				shadowFactor[0] = CalculateShadowFactor(samShadow,g_shadowMap,pin.shadowTex);
		}

		[unroll]
		for(int i=0; i<numLights; ++i)
		{
			float4 A, D, S;
			ComputeDirLight(g_material, g_dirLights[i], normal, toEye, A, D, S);
			
			ambient += A;
			diffuse += D * shadowFactor[i];
			specular += S * shadowFactor[i];
		}

		//Modulate texture color with lighting
		color = color * (ambient + diffuse) + specular;
	}

	//Fog Effect
	if(fogEnable)
	{
		clip(g_fogStart + g_fogRange - dist);

		float factor = saturate((dist - g_fogStart) / g_fogRange);
		color = lerp(color, g_fogColor, factor);
	}

	//Reflection
	if(useReflection)
	{
		float3 refDir = reflect(-toEye, normal);
		float4 refColor = g_cubeMap.Sample(samplerTex, refDir);
		color = lerp(color, refColor, g_material.reflection);
	}

	color.a = texColor.a * g_material.diffuse.a;

	return color;
}

//All techniques
technique11 Light1
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,false,false,false,false,false,false,false,false)) );
	}
}

technique11 Light2
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,false,false,false,false,false,false,false,false)) );
	}
}

technique11 Light3
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,false,false,false,false,false,false,false,false)) );
	}
}

technique11 Light1NormalMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,false,false,true,false,false,false,false,false)) );
	}
}

technique11 Light2NormalMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,false,false,true,false,false,false,false,false)) );
	}
}

technique11 Light3NormalMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,false,false,true,false,false,false,false,false)) );
	}
}

technique11 Light1NormalParallaxMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,false,false,true,true,false,false,false,false)) );
	}
}

technique11 Light2NormalParallaxMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,false,false,true,true,false,false,false,false)) );
	}
}

technique11 Light3NormalParallaxMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,false,false,true,true,false,false,false,false)) );
	}
}


technique11 Light1Texture
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,true,false,false,false,false,false,false,false)) );
	}
}

technique11 Light2Texture
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,true,false,false,false,false,false,false,false)) );
	}
}

technique11 Light3Texture
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,true,false,false,false,false,false,false,false)) );
	}
}

technique11 Light1TexAlphaClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,true,true,false,false,false,false,false,false)) );
	}
}

technique11 Light2TexAlphaClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,true,true,false,false,false,false,false,false)) );
	}
}

technique11 Light3TexAlphaClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,true,true,false,false,false,false,false,false)) );
	}
}

technique11 Light1TexNormalMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,true,false,true,false,false,false,false,false)) );
	}
}

technique11 Light2TexNormalMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,true,false,true,false,false,false,false,false)) );
	}
}

technique11 Light3TexNormalMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,true,false,true,false,false,false,false,false)) );
	}
}

technique11 Light1TexNormalParallaxMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,true,false,true,false,false,false,false,false)) );
	}
}

technique11 Light2TexNormalParallaxMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,true,false,true,false,false,false,false,false)) );
	}
}

technique11 Light3TexNormalParallaxMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,true,false,true,true,false,false,false,false)) );
	}
}

technique11 Light1TexShadowMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,true,false,false,false,true,false,false,false)) );
	}
}

technique11 Light2TexShadowMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,true,false,false,false,true,false,false,false)) );
	}
}

technique11 Light3TexShadowMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,true,false,false,false,true,false,false,false)) );
	}
}

technique11 Light1TexPCFShadowMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,true,false,false,false,true,true,false,false)) );
	}
}

technique11 Light2TexPCFShadowMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,true,false,false,false,true,true,false,false)) );
	}
}

technique11 Light3TexPCFShadowMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,true,false,false,false,true,true,false,false)) );
	}
}

technique11 Light1TexRefelction
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,true,false,false,false,false,false,true,false)) );
	}
}

technique11 Light2TexRefelction
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,true,false,false,false,false,false,true,false)) );
	}
}

technique11 Light3TexRefelction
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,true,false,false,false,false,false,true,false)) );
	}
}

technique11 Light1TexFog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,true,false,false,false,false,false,false,true)) );
	}
}

technique11 Light2TexFog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,true,false,false,false,false,false,false,true)) );
	}
}

technique11 Light3TexFog
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,true,false,false,false,false,false,false,true)) );
	}
}

technique11 Light1TexAlphaClipNormaMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(1,true,true,true,false,false,false,false,false)) );
	}
}

technique11 Light2TexAlphaClipNormaMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(2,true,true,true,false,false,false,false,false)) );
	}
}

technique11 Light3TexAlphaClipNormaMapping
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS(3,true,true,true,false,false,false,false,false)) );
	}
}