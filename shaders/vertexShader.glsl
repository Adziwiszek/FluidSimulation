#version 330 core
layout (location = 0) in vec3 inPosition;

uniform mat4 proj;
uniform vec3 offset;

void main()
{
  vec3 pos = inPosition + offset;
  gl_Position = proj * vec4(pos, 1.0);
}
