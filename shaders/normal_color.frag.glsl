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
  vec3 n;
  if (u_has_texture_normal != 0)
  {
    // Sample the normal from normal texture as color which is in range [0, 1].
    // Note: this vector is in tangent space.
    vec3 normal_sampled = texture(u_texture_normal, vs_out_texcoord).rgb;
    // Transform the sampled normal vector from tangent space to world space
    n = normalize(vs_out_TBN * normal_sampled);
  } 
  else 
  {
    n = normalize(vs_out_normal_world_space);
  }
  
  fs_out_color = vec4(n, 1.0);
}
