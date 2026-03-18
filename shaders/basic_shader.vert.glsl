#version 460 core

layout(location = 0) in vec3 in_local_pos;

uniform mat4 mat_transform;
uniform mat4 mat_ftc;
uniform mat4 mat_cam;
uniform mat4 mat_per;

void main()
{
  vec4 local_pos = vec4(in_local_pos, 1.0f);
  // apply transform
  local_pos = mat_transform * local_pos;
  // frame to canonical space
  vec4 global_pos = mat_ftc * local_pos;
  // canonical to camera space
  vec4 camera_pos = mat_cam * global_pos;
  // camera to clip space (camera to perspective)
  vec4 clip_pos = mat_per * camera_pos;
  gl_Position = vec4(clip_pos);
}
