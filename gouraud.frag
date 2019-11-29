// Phong fragment shader phong-tex.frag matched with phong-tex.vert
#version 330

// Some drivers require the following
precision highp float;


in vec4 out_Color;
layout(location = 0) out vec4 gl_FragColor;

in vec3 in_Position;
in vec3 in_Normal;
 
void main(void) {
    
	gl_FragColor = out_Color;


}