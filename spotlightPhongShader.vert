// spotlightPhongShader.vert
#version 330

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 generalLightPos;
uniform vec3 reflectorPosition;
uniform vec3 reflectorNormal;

//normal mapping
layout(location = 4) in vec4 tangent;


in vec3 in_Pos;
in vec3 in_Normal;

out vec3 lightDirection;
out vec3 ex_NormalWorld;
out vec3 ex_Pos;
out vec3 ex_V;
out vec3 ex_L;

out vec3 eyeTan;
out vec3 lightTan;


in vec2 in_TexCoord;
out vec2 ex_TexCoord;

void main(void) { 
	ex_Pos = vec3(model * vec4(in_Pos,1.0));
	vec4 vertexPosition = (model) * vec4(in_Pos,1.0); //using for normal mapping

	mat3 normalmatrix = transpose(inverse(mat3(model)));  
	ex_NormalWorld =  normalize(normalmatrix * in_Normal);
	lightDirection = reflect(generalLightPos - reflectorPosition, reflectorNormal);
	lightDirection.x = -lightDirection.x;

	vec3 worldPos = (model * vec4(in_Pos,1.0)).xyz; //added during normal mapping

	ex_TexCoord = in_TexCoord;

    gl_Position = projection * view * vec4(ex_Pos,1.0);
	
	ex_V = normalize(-vertexPosition).xyz;
	ex_L = normalize(reflectorPosition.xyz - vertexPosition.xyz);


	//normal mapping
	vec3 tan = (normalmatrix * tangent.xyz);
	vec3 bitan = cross(ex_NormalWorld,tan)* tangent.w;
	
	mat3 TBN = transpose(mat3(tan, bitan, ex_NormalWorld));
	eyeTan = TBN * ex_V;
	lightTan = TBN * ex_L;

}