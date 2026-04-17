#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

layout(location=0) out vec2 outUV;

layout(set=0,binding=0)
uniform SceneConstants
{
  mat4 mtxWorld;
  mat4 mtxView;
  mat4 mtxProj;
};

void main() {
  vec4 worldPosition = mtxWorld * vec4(inPos, 1.0);
  gl_Position = mtxProj * mtxView * worldPosition;
  outUV = inUV;
}
