#version 460 core

in vec2 vs_out_texcoord;
in vec3 vs_out_normal_world_space;
in vec3 vs_out_frag_world_space;
in mat3 vs_out_TBN;

out vec4 fs_out_color;

layout(binding = 0) uniform sampler2D u_texture_color;
 
void main()
{
	// Sample the surface color from the texture
  vec3 surface_color = texture(u_texture_color, vs_out_texcoord).rgb;
  
  fs_out_color = vec4(surface_color, 1.0);
}
