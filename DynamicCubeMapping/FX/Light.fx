//Light calculation

//Directional light
struct DirLight
{
	float4	ambient;
	float4	diffuse;
	float4	specular;

	float3	dir;
	float	unused;		//Not used, only for "4D Vector" alignment
};

//point light
struct PointLight
{
	float4	ambient;
	float4	diffuse;
	float4	specular;

	float3	pos;
	float	range;

	float3	att;		//Attenuation factors
	float	unused;		//Not used, only for "4D Vector" alignment
};

//Spot light
struct SpotLight
{
	float4	ambient;
	float4	diffuse;
	float4	specular;

	float3	dir;
	float	range;

	float3	pos;		//Position of spot light
	float	spot;		//spot factor of the light

	float3	att;		//Attenuation factors
	float	theta;		//Max divergence angle
};

struct Material
{
	float4	ambient;
	float4	diffuse;
	float4	specular;	//The 4th element in 'specular' is the specular factor "n" for the material surface
	float4	reflection;
};

//Directional light calculation
void ComputeDirLight(Material mat,			//Material
					DirLight dirLight,		//directional light
					float3 normal,			//vertex normal
					float3 toEye,			//Vector from vertex to eye
					out float4 ambient,		//Outputs: ambient, diffuse, specular
					out float4 diffuse,	
					out float4 specular)
{
	ambient  = float4(0.0f,0.0f,0.f,0.f);
	diffuse  = float4(0.f,0.f,0.f,0.f);
	specular = float4(0.f,0.f,0.f,0.f);

	//Ambient result
	ambient = mat.ambient * dirLight.ambient;

	float diffFactor = -dot(normal,dirLight.dir);

	[flatten]
	if(diffFactor > 0)
	{
		//Diffuse result
		diffuse = mat.diffuse * dirLight.diffuse * diffFactor;

		float3 refLight = reflect(dirLight.dir,normal);
		float specFactor = pow(max(dot(refLight,toEye),0.f),mat.specular.w);
		specular = mat.specular * dirLight.specular * specFactor;
	}
}

//Point light calculation
void ComputePointLight(Material mat,
						PointLight pLight,
						float3 normal,
						float3 position,		//Vertex position
						float3 toEye,
						out float4 ambient,		//Outputs
						out float4 diffuse,
						out float4 specular)
{
	ambient  = float4(0.f,0.f,0.f,0.f);
	diffuse	 = float4(0.f,0.f,0.f,0.f);
	specular = float4(0.f,0.f,0.f,0.f);

	//Light direction: from vertex to light source
	float3 dir = pLight.pos - position;

	//Distance from light source to vertex
	float dist = length(dir);

	//If out of range, do not calculate
	if(dist > pLight.range)
		return;

	//Normalization
	dir /= dist;

	//Attenuation
	float att = 1/(pLight.att.x + pLight.att.y*dist + pLight.att.z*dist*dist);

	//Ambient result
	ambient = mat.ambient * pLight.ambient * att;

	float diffFactor = dot(dir,normal);
	if(diffFactor > 0)
	{
		//Diffuse result
		diffuse = mat.diffuse * pLight.diffuse * diffFactor * att;

		float3 refLight = reflect(-dir,normal);
		//Specular factor
		float specFactor = pow(max(dot(refLight,toEye),0.f),mat.specular.w);

		//Specular result
		specular = mat.specular * pLight.specular * specFactor * att;
	}
}

//Spot light calculation
void ComputeSpotLight(Material mat,
					SpotLight L,
					float3 normal,
					float3 position,
					float3 toEye,
					out float4 ambient,
					out float4 diffuse,	
					out float4 specular)
{
	ambient  = float4(0.f,0.f,0.f,0.f);
	diffuse  = float4(0.f,0.f,0.f,0.f);
	specular = float4(0.f,0.f,0.f,0.f);

	float3 dir = L.pos - position;
	float dist = length(dir);

	if(dist > L.range)
		return;

	dir /= dist;
	
	float att = 1/(L.att.x + L.att.y*dist + L.att.z*dist*dist);

	float tmp = -dot(dir,L.dir);
	if(tmp < cos(L.theta))
		return;
	float spotFactor = pow(max(tmp,0.f),L.spot);

	ambient = mat.ambient * L.ambient * spotFactor;

	float diffFactor = dot(dir,normal);

	if(diffFactor > 0)
	{
		diffuse = mat.diffuse * L.diffuse * diffFactor * att * spotFactor;
		
		float3 refLight = reflect(-dir,normal);
		float specFactor = pow(max(dot(refLight,toEye),0.f),mat.specular.w);
		specular = mat.specular * L.specular * specFactor * att * spotFactor;
	}
}

static const float SHADOW_MAP_SIZE = 2048.f;
static const float SHADOW_MAP_DX = 1/SHADOW_MAP_SIZE;

float CalculateShadowFactor(SamplerComparisonState samShadow, Texture2D shadowMap, float4 texPosH)
{
	texPosH.xyz /= texPosH.w;
	float depth = texPosH.z;

	return shadowMap.SampleCmpLevelZero(samShadow,texPosH.xy,depth).r;
}

float CalculateShadowFactor3x3(SamplerComparisonState samShadow, Texture2D shadowMap, float4 texPosH)
{
	texPosH.xyz /= texPosH.w;
	float depth = texPosH.z;

	const float dx = SHADOW_MAP_DX;
	const float2 offsets[9] = 
	{
		float2(-dx,-dx), float2(0,-dx), float2(dx,-dx),
		float2(-dx,0), float2(0,0), float2(dx,0),
		float2(-dx,dx), float2(0, dx), float2(dx,dx)
	};

	float factor = 0.f;
	[unroll]
	for(int i=0; i<9; ++i)
	{
		factor += shadowMap.SampleCmpLevelZero(samShadow,texPosH.xy+offsets[i], depth).r;
	}

	return factor / 9;
}