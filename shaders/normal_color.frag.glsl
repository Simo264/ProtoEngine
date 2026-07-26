#version 460 core

in vec2 vs_out_texcoord;
in vec3 vs_out_normal_world_space;
in vec3 vs_out_frag_world_space;
in mat3 vs_out_TBN;

out vec4 fs_out_color;

layout(location = 0) uniform vec3 u_camera_eye;
// layout(location = 1) uniform int  u_has_texture_albedo;
layout(location = 2) uniform int  u_has_texture_normal;

// material uniforms
layout(location = 5) uniform vec3 u_albedo;

// layout(binding = 0) uniform sampler2D u_texture_albedo;
layout(binding = 1) uniform sampler2D u_texture_normal;

vec3 linear_to_srgb(vec3 color, float gamma) 
{
  return pow(color, vec3(1.0f / gamma));
}

void main()
{
  vec3 n;
  if (u_has_texture_normal != 0)
  {
    // Sample the normal from normal texture as color which is in range [0, 1].
    vec3 normal_sampled = texture(u_texture_normal, vs_out_texcoord).rgb;
    vec3 normal_tangent = normal_sampled * 2.0 - 1.0; // from [0, 1] to [-1, 1]
    // Transform the sampled normal vector from tangent space to world space
    n = normalize(vs_out_TBN * normal_tangent);
  } 
  else 
  {
    n = normalize(vs_out_normal_world_space);
  }

  // We remap the World Space vector [-1, 1] in the range [0, 1] to be able to see it as a color
  vec3 normal_visual = n * 0.5 + 0.5;

  // We apply the final sRGB Encoding to send the color to the monitor  
  fs_out_color = vec4(linear_to_srgb(n, 2.2), 1.0);
}
