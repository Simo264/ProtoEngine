#version 460 core

in vec2 vs_out_texcoord;
in vec3 vs_out_normal;
in vec3 vs_out_frag_global_space;

out vec4 fs_out_color;

layout(binding=0) uniform sampler2D u_texture;

#define PI 3.1415926535897932384626433832795
 
void main()
{
  vec3 surface_color = texture(u_texture, vs_out_texcoord).rgb;
  
	// map normals from [-1, +1] to [0, 1]
	vec3 n = normalize(vs_out_normal);
	// n = n*0.5 + 0.5;
  
  // define the point light source
  vec3 light_position = vec3(0.0f, 1.5f, +1.5f);
  float light_power_watt = 100.0f;

  // calculate the general formula for the irradiance due a point source, E:
  // E 	= (P / 4pi)(cos(theta) / r^2)
  // 		= I * (cos(theta) / r^2)
  // 
  // Where:
  // 
  // I = P / 4pi 
  // cos(theta) = n dot l
  
  float r = length(light_position - vs_out_frag_global_space);
  vec3 light_dir = normalize(light_position - vs_out_frag_global_space);
  float angle_incidence = max(dot(n, light_dir), 0.0); 
  
  float intensity = light_power_watt / (4*PI);
  float irradiance = intensity * (angle_incidence / (r*r));
  
  // Lambertian shading:
  // The very simplest kind of reflection is a surface that reflects light equally to all directions, 
  // regardless of where it came from, so that the reflected light `L_r` is simply a constant multiple of the irradiance:
  // 
  // L_r = k*E
  //
  // The surface is described by its reflactance R, which is the fraction of the irradiance it reflects.
  // The coefficient relating reflected to incident light is R/pi :
  //
  // L_r = (R / pi)E
  // 
  // The reflectance can be different for different colors of light, and for simple modeling of color, 
  // it suffices to just keep three different reflectances, one each for red, green, and blue, 
  // so this shading equation is carried out separately for the three color channels.
  vec3 reflactance = surface_color;
  vec3 light_reflected = (reflactance / PI) * irradiance;
  fs_out_color = vec4(light_reflected, 1.0);
}
