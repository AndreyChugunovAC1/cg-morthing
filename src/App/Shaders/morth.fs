#version 330 core

out vec4 out_col;

uniform int mode;

in vec3 vert_col;

void main()
{
  if (mode == 1) {
    out_col = vec4(0.9, 0.9, 0.1, 1.0);
  } else if (mode == 2) {
    out_col = vec4(0.2, 0.1, 0.9, 1.0);
  } else {
    out_col = vec4(vert_col, 1.0);
  }
}