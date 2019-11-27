// spotlightPhongShader.frag
#version 330

precision highp float;

struct lightStruct {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct materialStruct {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform lightStruct light;
uniform vec4 lightPosition;
uniform float lightCutOff;
uniform materialStruct material;
uniform sampler2D textureUnit0;
uniform sampler2D normalMap; //added by Axel
uniform vec3 viewPos;

in vec2 ex_TexCoord;
in vec3 ex_NormalWorld;
in vec3 ex_Pos;
in vec3 lightDirection;
in vec3 ex_V;
in vec3 ex_L;
in vec3 ex_N;
in vec3 eyeTan;
in vec3 lightTan;

layout(location = 0) out vec4 out_Color;

void main (void){
	vec3 N = normalize((texture( normalMap, ex_TexCoord ).rgb-0.5) * 2 ); //added by axel
	//or
	//vec3 N = 2.0 * texture( normalMap, ex_TexCoord,-1.0).rgb -1.0 ; //added by axel
	//N = normalize(N);

	vec3 LightDir = normalize(ex_Pos - lightPosition.xyz);
	float theta = dot(LightDir, normalize(lightDirection));
	
		vec4 ambientI = light.ambient * material.ambient;
		vec4 diffuseI = light.diffuse * material.diffuse;
		diffuseI = diffuseI * max(dot(N,normalize(lightTan)),0);

		vec3 viewDir = normalize(viewPos - ex_Pos);
		vec3 R = reflect(lightTan,N);

		vec4 specularI = light.specular * material.specular;
		specularI = specularI * pow(max(dot(eyeTan, R),0.0), material.shininess);
	
	if (theta > lightCutOff){
		out_Color = (ambientI + diffuseI + specularI) * texture(textureUnit0, ex_TexCoord);
	}
	else
	{
		out_Color = vec4(light.ambient.xyz * vec3(texture (textureUnit0, ex_TexCoord)), 1.0);
		}
	
	//out_Color = (ambientI + diffuseI + specularI) * texture(textureUnit0, ex_TexCoord);
	//out_Color = vec4(N, 1.0);
}