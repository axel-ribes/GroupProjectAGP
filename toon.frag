// textured.frag
#version 330

// Some drivers require the following
precision highp float;

struct lightStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct materialStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform lightStruct light;
uniform materialStruct material;

uniform sampler2D textureUnit0;
uniform sampler2D tex; 
in vec2 ex_TexCoord; 

in vec3 ex_N;
in vec3 ex_V;
in vec3 ex_L;
in float ex_dist;


uniform float attenuationConstant;
uniform float attenuationLinear;
uniform float attenuationQuadratic;

layout(location = 0) out vec4 out_colour;

 
void main(void) {

// Ambient intensity	
	vec4 ambientI = light.ambient * material.ambient;											

	
	vec4 diffuseI = light.diffuse * material.diffuse;

	diffuseI = diffuseI * max(dot(normalize(ex_N),normalize(ex_L)),0);

	// Specular intensity
	// Calculate R - reflection of light
	vec3 R = normalize(reflect(normalize(-ex_L),normalize(ex_N)));

	vec4 specularI = light.specular * material.specular;
	specularI = specularI * pow(max(dot(R,ex_V),0), material.shininess);

	//Attenuation
	float attenuation =  1.0f / (attenuationConstant + attenuationLinear * ex_dist + attenuationQuadratic * ex_dist * ex_dist);

	vec4 temp = diffuseI + specularI;

	vec4 colourtmp = ambientI + vec4(temp.rgb * attenuation, 1.0);

	float tc = max(dot(normalize(ex_N),normalize(ex_L)),0);
	vec2 toon_tc = vec2(tc, 1.0);
	vec4 litColour = texture(tex, toon_tc);

	out_colour = litColour;
}