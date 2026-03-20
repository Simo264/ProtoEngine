#version 460 core

layout(location = 0) in vec3 in_local_pos;
layout(location = 1) in vec2 in_tex_coord;

uniform mat4 mat_transform;
uniform mat4 mat_ftc;
uniform mat4 mat_cam; 
uniform mat4 mat_per;

out vec2 vs_out_texcoord;
 
void main()
{
  vec4 v_local_space = vec4(in_local_pos, 1.0f);
  // apply transform
  v_local_space = mat_transform * v_local_space;
  // frame to canonical space
  vec4 v_global_space = mat_ftc * v_local_space;
  // canonical to camera space
  vec4 v_camera_space = mat_cam * v_global_space;
  // camera to clip space (camera to perspective)
  vec4 v_clip_space = mat_per * v_camera_space;
  
  gl_Position = v_clip_space;
  vs_out_texcoord = in_tex_coord;
}
