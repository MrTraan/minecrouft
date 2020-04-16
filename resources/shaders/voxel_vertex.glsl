#version 330 core
layout (location = 0) in uint x;
layout (location = 1) in uint z;
layout (location = 2) in uint y;
layout (location = 3) in uint aTexIndex;
layout (location = 4) in uint aTexX;
layout (location = 5) in uint aTexY;

out float texIndex;
out float texX;
out float texY;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 position = vec4(float(x) / 10.0, float(y) / 10.0, float(z) / 10.0, 1.0);
    gl_Position = projection * view * model * position;
	texIndex = float(aTexIndex);
	texX = float(aTexX);
	texY = float(aTexY);
} 
