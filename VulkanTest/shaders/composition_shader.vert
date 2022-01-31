#version 450

const vec2 QUAD_VERTICES[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

void main() 
{
	gl_Position = vec4(QUAD_VERTICES[gl_VertexIndex], 1.0, 1.0);
}