#version 460 core

in vec2 vs_out_texcoord;
in vec3 vs_out_normal_world_space;
in vec3 vs_out_frag_world_space;
in mat3 vs_out_TBN;

out vec4 fs_out_color;

layout(location = 0) uniform vec3 u_camera_eye;
layout(location = 1) uniform int  u_has_texture_albedo;
layout(location = 2) uniform int  u_has_texture_normal;

// material uniforms
layout(location = 5) uniform vec3 u_albedo;

layout(binding = 0) uniform sampler2D u_texture_albedo;
layout(binding = 1) uniform sampler2D u_texture_normal;

vec3 linear_to_srgb(vec3 color, float gamma)
{
  return pow(color, vec3(1.0f / gamma));
}

void main()
{
  vec3 surface_color = u_albedo;
  if (u_has_texture_albedo != 0)
    surface_color *= texture(u_texture_albedo, vs_out_texcoord).rgb;

  // We apply the final sRGB Encoding to send the color to the monitor
  fs_out_color = vec4(linear_to_srgb(surface_color, 2.2), 1.0);
}
