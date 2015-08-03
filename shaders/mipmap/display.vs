#version 430 core

const mat4 mvp = mat4
( 1, 0,  0,    -0,
  0, 1,  0,    -0,
  0, 0, -0.01, -0,
  0, 0,  0,     1 );

layout(location=0) in vec4 in_vertex;
layout(location=1) in vec2 in_texture;

out vec2 texcoord;

void main()
{
  texcoord = in_texture;
  gl_Position = mvp * in_vertex;
}
