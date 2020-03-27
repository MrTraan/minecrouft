#version 330 core
out vec4 FragColor;  

in vec2 TexCoord;
in float TexIndex; 

uniform sampler2DArray ourTexture;
  
void main()
{
    vec3 texturePos = vec3(TexCoord, TexIndex);

	float alpha = 6*texture( ourTexture, texturePos ).a;
    alpha -= texture( ourTexture, texturePos + vec3( 0.001f, 0.0f, 0.0f ) ).a;
    alpha -= texture( ourTexture, texturePos + vec3( -0.001f, 0.0f, 0.0f ) ).a;
    alpha -= texture( ourTexture, texturePos + vec3( 0.0f, 0.001f, 0.0f ) ).a;
    alpha -= texture( ourTexture, texturePos + vec3( 0.0f, -0.001f, 0.0f ) ).a;
    alpha -= texture( ourTexture, texturePos + vec3( 0.0f, 0.0f, 0.001f ) ).a;
    alpha -= texture( ourTexture, texturePos + vec3( 0.0f, 0.0f, -0.001f ) ).a;
	
    FragColor = vec4( 1.0f, 1.0f, 1.0f, alpha );
}
