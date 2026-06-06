#version 460 core

in vec2 vs_out_texcoord;
in vec3 vs_out_normal_world_space;
in vec3 vs_out_frag_world_space;
in mat3 vs_out_TBN;

out vec4 fs_out_color;

layout(binding = 0) uniform sampler2D u_texture_color;
layout(binding = 1) uniform sampler2D u_texture_normal;

layout(location = 0) uniform vec3 u_camera_eye;

void main()
{
  // vec3 n = normalize(vs_out_normal_world_space);
  // vec3 color_normal = n * 0.5 + vec3(0.5);
  
  vec3 color_normal = texture(u_texture_normal, vs_out_texcoord).rgb;
  
  fs_out_color = vec4(color_normal, 1.0);
}
