#version 450
#extension GL_ARB_separate_shader_objects : enable

//attribute
layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec4 vColor;

layout(location = 0) out vec4 out_Color;

//uniforms

void main()
{
	//code
	gl_PointSize = 14.0;
	
	gl_Position =  vec4(vPosition.xy, 1.0, 1.0);
	out_Color = vColor;
}
