#version 460 core

in vec2 vs_out_texcoord;
out vec4 fs_out_color;

layout(binding=0) uniform sampler2D u_texture;

void main()
{    
  fs_out_color = texture(u_texture, vs_out_texcoord);
}
