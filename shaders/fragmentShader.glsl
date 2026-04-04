#version 330 core

in vec2 UV;
out vec4 fragColor;

uniform sampler2D smokeMap;

void main()
{
  float smoke = texture(smokeMap, UV).r;
  fragColor = vec4(smoke, smoke, smoke, 1.0f);
}
