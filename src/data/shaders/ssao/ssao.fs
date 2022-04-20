#version 100

varying vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform mat4 projection;

void main()
{
    gl_FragColor = occlusion;
}
