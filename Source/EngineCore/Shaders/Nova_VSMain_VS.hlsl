// -----------------------------------------------------------
// File: Nova_VSMain_VS.hlsl
// The vertex shader file for the Nova Engine.
// -----------------------------------------------------------

// -----------------------------------------------------------
// Globals
// -----------------------------------------------------------
cbuffer cbMatrices : register( b0 )
{
	matrix g_mWorldViewProjection : packoffset( c0 );
	matrix g_mWorld				  : packoffset( c4 );
};

cbuffer cbLights : register(b1)
{
	float4 g_LightDiffuse[8];			// Light's diffuse color - we can support up to eight lights with this shader.
	float4 g_LightDir[8];				// Light's direction in world space
	float4 g_fAmbient;
	int	   g_nNumLights;
};

cbuffer cbObjectColors : register(b2)
{
	float4 g_vDiffuseObjectColor : packoffset( c0 );
	float4 g_vAmbientObjectColor : packoffset( c1 );
	bool g_bHasTexture			 : packoffset( c2.x );
};

// ------------------------------------------------------------
// Input / Output structures
// ------------------------------------------------------------

struct VS_INPUT
{
	float4 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 vDiffuse : COLOR0;
	float2 vTexcoord : TEXCOORD0;
	float4 vPosition : SV_POSITION;
};

// Vertex Shader
// ------------------------------------------------------------
VS_OUTPUT Nova_VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;

	float3 vNormalWorldSpace;
	float dotProduct;
	float4 dottedLightColor;

	Output.vPosition = mul( Input.vPosition, g_mWorldViewProjection );
	vNormalWorldSpace = mul( Input.vNormal, (float3x3)g_mWorld );
	Output.vTexcoord = Input.vTexcoord;

	// Compute simple directional lighting equation
	float4 vTotalLightDiffuse = float4(0, 0, 0, 0);
	for (int i = 0; i < g_nNumLights; i++)
	{
		dotProduct = dot(vNormalWorldSpace, g_LightDir[i]);
		dotProduct = max(0, dotProduct);
		dottedLightColor = g_LightDiffuse[i] * dotProduct;
		vTotalLightDiffuse += dottedLightColor;
	}

	Output.vDiffuse.rgb = g_vDiffuseObjectColor * vTotalLightDiffuse + g_vAmbientColorObjectColor * g_fAmbient;

	Output.vDiffuse.a = g_vDiffuseObjectColor.a;

	return Output;
}