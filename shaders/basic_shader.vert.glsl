#version 460 core

layout(location = 0) in vec3 in_pos; 			// in local coordinate system
layout(location = 1) in vec3 in_normal; 	// in local coordinate system
layout(location = 2) in vec2 in_texcoord; // in local coordinate system
layout(location = 3) in vec3 in_tangent; 	// in tangent coordinate system

uniform mat4 mat_transform;
uniform mat4 mat_ftc;
uniform mat4 mat_cam; 
uniform mat4 mat_per;

out vec2 vs_out_texcoord;
out vec3 vs_out_normal_world_space;
out vec3 vs_out_frag_world_space;
out mat3 vs_out_TBN;
 
void main()
{
	// apply transform
  vec4 p_local_space = mat_transform * vec4(in_pos, 1.0f);
  // frame to canonical space
  vec4 p_world_space = mat_ftc * p_local_space;
  vs_out_frag_world_space = vec3(p_world_space);
  // canonical to camera space
  vec4 p_camera_space = mat_cam * p_world_space;
  // camera to clip space (camera to perspective)
  vec4 p_clip_space = mat_per * p_camera_space;
  
  // normal transformation for non-uniform scaling: N = (M^-1)^T
  mat3 normal_matrix = transpose(inverse(mat3(mat_ftc * mat_transform)));
  vec3 normal_world_space = normal_matrix * in_normal;
  
  // normal mapping: calculate the TBN matrix to transform normals extracted from the texture into model space
  vec3 N = normalize(normal_world_space);
  vec3 T = normalize(normal_matrix * in_tangent);
  T = normalize(T - dot(T, N) * N);
  vec3 B = normalize(cross(normal_world_space, T));
  vs_out_TBN = mat3(T, B, normal_world_space);
  
  gl_Position = p_clip_space;
  vs_out_texcoord = in_texcoord;
  vs_out_normal_world_space = normal_world_space;
}
