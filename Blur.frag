#version 330
precision highp float;

uniform sampler2D textureUnit0;
uniform sampler2D textureUnit1;
uniform sampler2D textureUnit2;
uniform sampler2D textureUnit3;
uniform sampler2D textureUnit4;
uniform sampler2D textureUnit5;

in vec2 ex_TexCoord;
layout(location = 0) out vec4 out_Color;
vec4 texture;
 
void main(void) 
{

	vec4 tex0 = texture(textureUnit0, ex_TexCoord);
	vec4 tex1 = texture(textureUnit1, ex_TexCoord);
	vec4 tex2 = texture(textureUnit2, ex_TexCoord);
	vec4 tex3 = texture(textureUnit3, ex_TexCoord);
	vec4 tex4 = texture(textureUnit4, ex_TexCoord);
	vec4 tex5 = texture(textureUnit5, ex_TexCoord);
		

	texture = (tex0 + tex1 + tex2 + tex3 + tex4 + tex5) / 6;

	out_Color = texture;
}

//INVERSE COLOURS
//out_Color = vec4(0.5 - texture.x, 0.5 - texture.y, 1.0 - texture.z, 1.0);