#version 330 core

uniform sampler2D tex_2d;

uniform float dotLightAngle;
uniform float dotLightHeight;
uniform bool enableDotLight;
uniform float spotLightLatitude;
uniform float spotLightLongitude;
uniform bool enableSpotLight;

in vec3 vert_norm;
in vec2 vert_tex;
in vec3 vert_userPos;
in vec3 vert_point_pos;

out vec4 out_col;

void main()
{
	vec4 texel = texture(tex_2d, vert_tex);
	vec3 to_user = normalize(vert_userPos - vert_point_pos);
	vec3 reflected_to_user = 2 * vert_norm * dot(vert_norm, to_user) - to_user;

	vec3 color = texel.rgb * 0.3; // ka

	// spot light:
	vec3 spot_light_dir = vec3(
		sin(spotLightLatitude) * cos(spotLightLongitude), 
		-cos(spotLightLatitude), 
		sin(spotLightLatitude) * sin(spotLightLongitude)
	);

	if(enableSpotLight) // mb faster to mul by 0
	{
		color += vec3(0.04, 0.17, 0.79) * 0.1 // kd
			* clamp(abs(dot(-spot_light_dir, vert_norm)), 0.0, 1.0);
		color += vec3(0.69, 0.09, 0.65) * 1.2 // ks
			* pow(clamp(abs(dot(-spot_light_dir, reflected_to_user)), 0.0, 1.0), 
					11); // alpha
	}


	// dot light:
	vec3 dot_light_pos = vec3(20, 2 * dotLightHeight, 20);
	vec3 dot_light_dir = normalize(-dot_light_pos);
	float halfAngleCos = cos(dotLightAngle / 2.0f);
	vec3 to_light = normalize(dot_light_pos - vert_point_pos);

	if(enableDotLight) 
	{
		float cosangle = clamp(abs(dot(to_light, reflected_to_user)), 0.0, 1.0);
		if (cosangle > halfAngleCos) 
		{
			color += vec3(0.24, 0.78, 0.69) * 0.3 // kd
				* clamp(abs(dot(to_light, vert_norm)), 0.0, 1.0);
			color += vec3(0.23, 0.67, 0.16) * 0.3 // ks
				* pow(cosangle, 13); // alpha
		}
	}

	out_col = vec4(clamp(color, vec3(0), vec3(1)), 1);
	// out_col = vec4(texel.rgb, 1);
}