// phongShader.frag
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
uniform materialStruct material;
uniform vec3 viewPos;

in vec2 ex_TexCoord;

in vec3 ex_N;
in vec3 ex_Pos;

layout(location = 0) out vec4 out_Color;

void main (void){
	vec3 ex_L = normalize(lightPosition.xyz - ex_Pos);
	vec4 ambientI = light.ambient * material.ambient;
	vec4 diffuseI = light.diffuse * material.diffuse;
	diffuseI = diffuseI * max(dot(normalize(ex_N),normalize(ex_L)),0);

	vec3 viewDir = normalize(viewPos - ex_Pos);
	vec3 R = reflect(-ex_L,ex_N);

	vec4 specularI = light.specular * material.specular;
	specularI = specularI * pow(max(dot(viewDir, R),0.0), material.shininess);

	out_Color = (ambientI + diffuseI + specularI);
}