#version 330 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aTexIndex;

out vec2 TexCoord;
out float TexIndex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
	TexIndex = aTexIndex;
} 
