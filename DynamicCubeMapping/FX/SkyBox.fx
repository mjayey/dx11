
cbuffer PerObject
{
	float4x4 g_worldViewProj;
};

struct VertexIn
{
	float3 posL: POSITION;
};

struct VertexOut
{
	float4 posH: SV_POSITION;
	float3 posL: POSITION;
};

TextureCube g_cubeMap;

SamplerState samTexture
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.posL = vin.posL;
	vout.posH = mul(float4(vin.posL,1.f), g_worldViewProj).xyww;

	return vout;
}

float4 PS(VertexOut pin): SV_TARGET
{
	return g_cubeMap.Sample(samTexture,pin.posL);
}

RasterizerState CounterClockFrontRS
{
	FrontCounterClockwise = true;
};

DepthStencilState LessEqualDSS
{
    DepthFunc = LESS_EQUAL;
};

technique11 SkyBoxTech
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS()) );
		SetPixelShader( CompileShader(ps_5_0, PS()) );
        SetDepthStencilState(LessEqualDSS, 0);
		SetRasterizerState(CounterClockFrontRS);
	}
}