#version 330 core

in vec2 UV;
out vec4 fragColor;

uniform sampler2D smokeMap;
uniform sampler2D solidMap;

void main()
{
  float smoke = texture(smokeMap, UV).r;
  float solid = texture(solidMap, UV).r;
  if (solid > 0.0) {
    fragColor = vec4(0.0f, 0.2f, 0.9f, 1.0f);
  } else {
    fragColor = vec4(smoke, smoke, smoke, 1.0f);
  }
}
