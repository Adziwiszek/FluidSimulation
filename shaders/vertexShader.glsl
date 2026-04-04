#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 aUV;

uniform mat4 proj;

out vec2 UV;

void main()
{
  gl_Position = proj * vec4(pos, 1.0);
  UV = aUV;
}
