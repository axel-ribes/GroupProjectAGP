// phong-tex.vert
// Vertex shader for use with a Phong or other reflection model fragment shader
// Calculates and passes on V, L, N vectors for use in fragment shader, phong2.frag
#version 330

in  vec3 in_Position;
in  vec3 in_Normal;
out vec3 ex_N;
out vec3 ex_V;
out vec3 ex_L;

out float ex_dist;

in vec2 in_TexCoord;

vec2 ex_TexCoord;

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

uniform float attenuationConstant;
uniform float attenuationLinear;
uniform float attenuationQuadratic;

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 lightPosition;
//uniform mat3 normalmatrix;

out vec4 out_Color;

void main(void) {


	vec4 vertexPosition = modelview * vec4(in_Position,1.0);	

	ex_V =  normalize(-vertexPosition).xyz;	

	mat3 normalmatrix = transpose(inverse(mat3(modelview)));
	ex_N = normalize(normalmatrix * in_Normal);

	ex_L = normalize(lightPosition.xyz - vertexPosition.xyz);
	ex_dist = distance(vertexPosition, lightPosition);

    gl_Position = projection * vertexPosition;	

	// Ambient intensity	
	vec4 ambientI = light.ambient * material.ambient;											

	
	vec4 diffuseI = light.diffuse * material.diffuse;

	diffuseI = diffuseI * max(dot(normalize(ex_N),normalize(ex_L)),0);

	// Specular intensity
	// Calculate R - reflection of light
	vec3 R = normalize(reflect(normalize(-ex_L),normalize(ex_N)));

	vec4 specularI = light.specular * material.specular;
	specularI = specularI * pow(max(dot(R,ex_V),0), material.shininess);

	float attenuation =  1.0f / (attenuationConstant + attenuationLinear * ex_dist + attenuationQuadratic * ex_dist * ex_dist);

	vec4 temp = diffuseI + specularI;

	vec4 colour = ambientI + vec4(temp.rgb * attenuation, 1.0);

	vec4 out_Color = colour * texture(textureUnit0, ex_TexCoord);	

}