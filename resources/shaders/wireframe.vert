#version 460 core
layout (location = 0) in uint x;
layout (location = 1) in uint z;
layout (location = 2) in uint y;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
	vec4 position = vec4(float(x) / 10.0, float(y) / 10.0, float(z) / 10.0, 1.0);
    gl_Position = projection * view * model * position;
};
