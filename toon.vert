uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 lightPosition;
//uniform mat3 normalmatrix;

in  vec3 in_Position;
in  vec3 in_Normal;
out vec3 ex_N;
out vec3 ex_V;
out vec3 ex_L;

out float ex_dist;

in vec2 in_TexCoord;
out vec2 ex_TexCoord;


void main(void) {
	vec4 vertexPosition = modelview * vec4(in_Position,1.0);	

	ex_V =  normalize(-vertexPosition).xyz;	

	mat3 normalmatrix = transpose(inverse(mat3(modelview)));
	ex_N = normalize(normalmatrix * in_Normal);

	ex_L = normalize(lightPosition.xyz - vertexPosition.xyz);
	ex_dist = distance(vertexPosition, lightPosition);

	ex_TexCoord = in_TexCoord;

    gl_Position = projection * vertexPosition;	
}