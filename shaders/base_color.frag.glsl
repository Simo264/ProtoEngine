#version 460 core

in vec2 vs_out_texcoord;
in vec3 vs_out_normal_world_space;
in vec3 vs_out_frag_world_space;
in mat3 vs_out_TBN;

out vec4 fs_out_color;

layout(location = 0) uniform vec3 u_camera_eye;

// material uniforms
layout(location = 5) uniform vec3 u_surface_color;
layout(location = 6) uniform int  u_has_texture_color;
layout(location = 7) uniform int  u_has_texture_normal;

layout(binding = 0) uniform sampler2D u_texture_color;
layout(binding = 1) uniform sampler2D u_texture_normal;

void main()
{
  vec3 surface_color = u_surface_color;
  if (u_has_texture_color != 0)
    surface_color *= texture(u_texture_color, vs_out_texcoord).rgb;
  
  fs_out_color = vec4(surface_color, 1.0);
}
