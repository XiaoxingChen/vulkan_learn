#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform buffer
{
  mat4 mvp;
} uniformBuffer;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 inTexCoord;

layout (location = 0) out vec2 outTexCoord;

void main()
{
  outTexCoord = inTexCoord;
  // gl_Position = uniformBuffer.mvp * pos;
  gl_Position.x = -pos.x;
  gl_Position.y = pos.y;
  gl_Position.z = 0;
  gl_Position.w = 1;
}