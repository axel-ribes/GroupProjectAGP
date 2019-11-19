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

uniform lightStruct lightBlue;
uniform lightStruct lightYellow;
uniform vec4 lightBluePosition;
uniform vec4 lightYellowPosition;
uniform float lightCutOff;
uniform materialStruct material;
uniform sampler2D textureUnit0;
uniform vec3 viewPos;

in vec2 ex_TexCoord;
in vec3 ex_NormalWorld;
in vec3 ex_Pos;
in vec3 lightBlueDirection;
in vec3 lightYellowDirection;

layout(location = 0) out vec4 out_Color;

void main (void){
	vec3 LightDirBlue = normalize(ex_Pos - lightBluePosition.xyz);
	vec3 LightDirYellow = normalize(ex_Pos - lightYellowPosition.xyz);
	float thetaBlue = dot(LightDirBlue, normalize(lightBlueDirection));
	float thetaYellow = dot(LightDirYellow, normalize(lightYellowDirection));

	if (thetaBlue > lightCutOff){
		vec4 ambientI = lightBlue.ambient * material.ambient;
		vec4 diffuseI = lightBlue.diffuse * material.diffuse;
		diffuseI = diffuseI * max(dot(normalize(ex_NormalWorld),normalize(-LightDirBlue)),0);

		vec3 viewDir = normalize(viewPos - ex_Pos);
		vec3 R = reflect(LightDirBlue,ex_NormalWorld);

		vec4 specularI = lightBlue.specular * material.specular;
		specularI = specularI * pow(max(dot(viewDir, R),0.0), material.shininess);

		out_Color = (ambientI + diffuseI + specularI) * texture(textureUnit0, ex_TexCoord);
	}
	else if (thetaYellow > lightCutOff){
		vec4 ambientI = lightYellow.ambient * material.ambient;
		vec4 diffuseI = lightYellow.diffuse * material.diffuse;
		diffuseI = diffuseI * max(dot(normalize(ex_NormalWorld),normalize(-LightDirYellow)),0);

		vec3 viewDir = normalize(viewPos - ex_Pos);
		vec3 R = reflect(LightDirYellow,ex_NormalWorld);

		vec4 specularI = lightYellow.specular * material.specular;
		specularI = specularI * pow(max(dot(viewDir, R),0.0), material.shininess);

		out_Color = (ambientI + diffuseI + specularI) * texture(textureUnit0, ex_TexCoord);
	}
	else
		out_Color = vec4(lightBlue.ambient.xyz * vec3(texture (textureUnit0, ex_TexCoord)), 1.0);

}