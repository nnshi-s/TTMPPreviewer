$input a_position, a_normal, a_texcoord0, a_tangent, a_bitangent
$output v_view, v_normal, v_texcoord0, v_tangent, v_bitangent

/*
 * Copyright 2014-2016 Dario Manesku. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.sh"
#include "uniforms.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );

	v_view = u_camPos - mul(u_model[0], vec4(a_position, 1.0)).xyz;

	v_texcoord0 = a_texcoord0;

	// vec3 normal = a_normal * 2.0 - 1.0;
	v_normal = mul(u_model[0], vec4(a_normal.xyz, 0.0) ).xyz;
	v_tangent = mul(u_model[0], vec4(a_tangent.xyz, 0.0) ).xyz;
	v_bitangent = mul(u_model[0], vec4(a_bitangent.xyz, 0.0) ).xyz;
}
