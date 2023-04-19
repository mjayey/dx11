
cbuffer PerObject
{
	float4x4	g_lightViewProj;
	float4x4	g_texTrans;
};

struct VertexIn
{
	float3	pos:	POSITION;
	float3	normal: NORMAL;
	float2	tex:	TEXCOORD;
};

struct VertexOut
{
	float4 posH: SV_POSITION;
	float2 tex:  TEXCOORD;
};

Texture2D	g_texture;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

RasterizerState DepthBias
{
	DepthBias = 10000;
	DepthBiasClamp = 0.0f;
	SlopeScaledDepthBias = 1.f;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.posH = mul(float4(vin.pos,1.f),g_lightViewProj);
	vout.tex = mul(float4(vin.tex,0.f,1.f),g_texTrans).xy;

	return vout;
}

void PS(VertexOut pin)
{
	float4 texColor = g_texture.Sample(samLinear,pin.tex);
	clip(texColor.a - 0.1f);
}

technique11 ShadowMap
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0, VS()) );
		SetPixelShader( NULL );

		SetRasterizerState(DepthBias);
	}
}

technique11 ShadowMapAlphaClip
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_4_0, VS()) );
		SetPixelShader( CompileShader(ps_4_0, PS()) );

		SetRasterizerState(DepthBias);
	}
}