#version 330 core

layout(location=0) in vec3 pos1;
layout(location=1) in vec3 norm1;
layout(location=2) in vec3 pos2;
layout(location=3) in vec3 norm2;

uniform mat4 mvp;
uniform float time;

uniform float lerp;
uniform bool enableManual;

out vec3 vert_col;

void main() {
  float ik;

  if (enableManual) {
    ik = lerp;
  } else {
    ik = 0.5 + tan(time * 3);
  }

  vec3 pos = mix(pos1, pos2, ik);
  vec3 norm = mix(norm1, norm2, ik);
	gl_Position = mvp * vec4(pos, 1);
  vert_col = abs(normalize(norm));
}
