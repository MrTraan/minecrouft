#version 460 core
out vec4 FragColor;  

in float texIndex;
in float texX;
in float texY;

uniform sampler2DArray ourTexture;
  
void main()
{
	FragColor = texture(ourTexture, vec3(texX, texY, texIndex));
}
