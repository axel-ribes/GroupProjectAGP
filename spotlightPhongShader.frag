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
uniform vec3 viewPos;

in vec2 ex_TexCoord;
in vec3 ex_NormalWorld;
in vec3 ex_Pos;
in vec3 lightDirection;

layout(location = 0) out vec4 out_Color;

void main (void){
	vec3 LightDir = normalize(ex_Pos - lightPosition.xyz);
	float theta = dot(LightDir, normalize(lightDirection));

	if (theta > lightCutOff){
		vec4 ambientI = light.ambient * material.ambient;
		vec4 diffuseI = light.diffuse * material.diffuse;
		diffuseI = diffuseI * max(dot(normalize(ex_NormalWorld),normalize(-LightDir)),0);

		vec3 viewDir = normalize(viewPos - ex_Pos);
		vec3 R = reflect(LightDir,ex_NormalWorld);

		vec4 specularI = light.specular * material.specular;
		specularI = specularI * pow(max(dot(viewDir, R),0.0), material.shininess);

		out_Color = (ambientI + diffuseI + specularI) * texture(textureUnit0, ex_TexCoord);
	}
	else
		out_Color = vec4(light.ambient.xyz * vec3(texture (textureUnit0, ex_TexCoord)), 1.0);

}