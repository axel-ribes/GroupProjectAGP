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
uniform float lightCutOff; // angle which determine the radius of the spotlight
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
	// calculate the vectors between the fragment and the lights positions (here it is the center of the reflectors)
	vec3 LightDirBlue = normalize(ex_Pos - lightBluePosition.xyz);
	vec3 LightDirYellow = normalize(ex_Pos - lightYellowPosition.xyz);

	// calculate angle between the direction of the ligh and the vectors between the fragment and the lights positions
	float thetaBlue = dot(LightDirBlue, normalize(lightBlueDirection));
	float thetaYellow = dot(LightDirYellow, normalize(lightYellowDirection));

	if (thetaBlue > lightCutOff && thetaYellow > lightCutOff){ // if the fragment is in both of the spotlight (the spotlights are superimposed)
		
		// Convert Yellow light from RGB to RYB
		vec3 RGBYellow = lightYellow.diffuse.xyz;
		vec3 RYBYellow = lightYellow.diffuse.xyz;
		RYBYellow.x = RGBYellow.r - min(RGBYellow.r, RGBYellow.g);
		RYBYellow.y = (RGBYellow.g + min(RGBYellow.r, RGBYellow.g))/2;
		RYBYellow.z = (RGBYellow.b + RGBYellow.g - min(RGBYellow.r, RGBYellow.g))/2;

		// Caluclate the blending color in RYB
		vec3 combinedColorLightRYB = RYBYellow + lightBlue.diffuse.xyz;

		// convert the blending color from RYB to RGB
		vec3 combinedColorLightRGB = combinedColorLightRYB;
		combinedColorLightRGB.r = combinedColorLightRYB.x + combinedColorLightRYB.y - min(combinedColorLightRYB.y, combinedColorLightRYB.z);
		combinedColorLightRGB.g = combinedColorLightRYB.y + 2*min(combinedColorLightRYB.y, combinedColorLightRYB.z);
		combinedColorLightRGB.b = 2*(combinedColorLightRYB.z - min(combinedColorLightRYB.y, combinedColorLightRYB.z));

		vec4 coloredLight = normalize(vec4(combinedColorLightRGB, 1));

		// Calculate the light with the blending color
		vec4 ambientI = lightBlue.ambient * material.ambient; // The ambient property is the same for both light
		vec4 diffuseI = coloredLight * material.diffuse;
		diffuseI = diffuseI * max(dot(normalize(ex_NormalWorld),normalize(-LightDirBlue)),0) * max(dot(normalize(ex_NormalWorld),normalize(-LightDirYellow)),0);

		vec3 viewDir = normalize(viewPos - ex_Pos);
		vec3 R = reflect(LightDirBlue,ex_NormalWorld) * reflect(LightDirYellow,ex_NormalWorld);

		vec4 specularI = coloredLight * material.specular;
		specularI = specularI * pow(max(dot(viewDir, R),0.0), material.shininess);

		out_Color = (ambientI + diffuseI + specularI) * texture(textureUnit0, ex_TexCoord);


	}
	else if (thetaBlue > lightCutOff){ // if the fragment is only int the spotlight of the blue light
		vec4 ambientI = lightBlue.ambient * material.ambient;
		vec4 diffuseI = lightBlue.diffuse * material.diffuse;
		diffuseI = diffuseI * max(dot(normalize(ex_NormalWorld),normalize(-LightDirBlue)),0);

		vec3 viewDir = normalize(viewPos - ex_Pos);
		vec3 R = reflect(LightDirBlue,ex_NormalWorld);

		vec4 specularI = lightBlue.specular * material.specular;
		specularI = specularI * pow(max(dot(viewDir, R),0.0), material.shininess);

		out_Color = (ambientI + diffuseI + specularI)* texture (textureUnit0, ex_TexCoord);
	}
	else if (thetaYellow > lightCutOff){ // if the fragment is only int the spotlight of the yellow light
		vec4 ambientI = lightYellow.ambient * material.ambient;
		vec4 diffuseI = lightYellow.diffuse * material.diffuse;
		diffuseI = diffuseI * max(dot(normalize(ex_NormalWorld),normalize(-LightDirYellow)),0);

		vec3 viewDir = normalize(viewPos - ex_Pos);
		vec3 R = reflect(LightDirYellow,ex_NormalWorld);

		vec4 specularI = lightYellow.specular * material.specular;
		specularI = specularI * pow(max(dot(viewDir, R),0.0), material.shininess);

		out_Color = (ambientI + diffuseI + specularI)* texture (textureUnit0, ex_TexCoord);
	}
	else // if the fragment isn't in both spotlight
		out_Color = vec4(lightBlue.ambient.xyz * vec3(texture (textureUnit0, ex_TexCoord)), 1.0);
}