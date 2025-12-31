#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 norm;
layout(location=2) in vec2 tex;

uniform mat4 mvp;
uniform float time;
uniform vec3 userPos;

out vec2 vert_tex;
out vec3 vert_norm;
out vec3 vert_userPos;
out vec3 vert_point_pos;

void main() {
	vec3 newPos = pos - vec3(0, 10, 0);
	vec3 invPos = 600.0 * newPos / dot(newPos, newPos);
	vec3 t = newPos + norm * 0.01;
	vec3 invNorm = normalize(-t / dot(t, t) + invPos);
	float ik = 0.06 * (1 + sin(time * 6));

	vec3 curPos = mix(newPos, invPos, ik);
	vec3 curNorm = normalize(mix(invNorm, norm, ik));

	vert_tex = tex;
	vert_norm = normalize(curNorm);
	vert_userPos = userPos;
	vert_point_pos = curPos;
	gl_Position = mvp * vec4(curPos, 1);
}
