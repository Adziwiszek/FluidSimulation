#version 330 core

in vec2 UV;
out vec4 fragColor;

uniform sampler2D smokeMap;
uniform sampler2D solidMap;

void main()
{
  float smoke = texture(smokeMap, UV).r;
  float solid = texture(solidMap, UV).r;

  if (solid < 0.5) {
    fragColor = vec4(0.2, 0.2, 0.2, 1.0);
  } else {
    fragColor = vec4(smoke, smoke, smoke, 1.0);
  }
}
