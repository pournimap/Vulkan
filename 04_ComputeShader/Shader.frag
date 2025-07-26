#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 out_Color;

layout(location = 0) out vec4 FragColor;

void main()
{
	//code
	vec2 coord = gl_PointCoord - vec2(0.5);
	//FragColor = 
	FragColor = vec4(out_Color);
	//FragColor = vec4(1.0, 0.0, 0.0, 1.0);

}
