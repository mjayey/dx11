#include "Inputs.h"
#include "Effects.h"

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosNormal[2] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Basic32[3] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	 0, 24,D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Pos[1] = 
{
	{"POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosNormalTangentTex[4] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

ID3D11InputLayout* InputLayouts::pos(NULL);
ID3D11InputLayout* InputLayouts::posNormal(NULL);
ID3D11InputLayout* InputLayouts::basic32(NULL);
ID3D11InputLayout* InputLayouts::posNormalTagentTex(NULL);

bool InputLayouts::InitAll(ID3D11Device *device)
{
	if(!pos)
	{
		D3DX11_PASS_DESC pDesc;
		Effects::fxSkyBox->fxSkyBoxTech->GetPassByIndex(0)->GetDesc(&pDesc);
		if(FAILED(device->CreateInputLayout(InputLayoutDesc::Pos,1,pDesc.pIAInputSignature,pDesc.IAInputSignatureSize,&pos)))
			return false;
	}

	if(!posNormalTagentTex)
	{
		D3DX11_PASS_DESC pDesc;
		Effects::fxBasic->fxLight3TexTech->GetPassByIndex(0)->GetDesc(&pDesc);
		if(FAILED(device->CreateInputLayout(InputLayoutDesc::PosNormalTangentTex,4,pDesc.pIAInputSignature,pDesc.IAInputSignatureSize,&posNormalTagentTex)))
			return false;
	}

	return true;
}

void InputLayouts::ReleaseAll()
{
	SafeRelease(pos);
	SafeRelease(posNormalTagentTex);
}