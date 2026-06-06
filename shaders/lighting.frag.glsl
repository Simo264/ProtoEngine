#version 460 core

in vec2 vs_out_texcoord;
in vec3 vs_out_normal_world_space;
in vec3 vs_out_frag_world_space;
in mat3 vs_out_TBN;

out vec4 fs_out_color;

layout(binding = 0) uniform sampler2D u_texture_color;
layout(binding = 1) uniform sampler2D u_texture_normal;

layout(location = 0) uniform vec3 u_camera_eye;

// Define the point light source properties
layout(location = 3) uniform vec3 u_light_position;
layout(location = 4) uniform vec3 u_light_color;
layout(location = 5) uniform float u_light_power_watt;

#define PI 3.1415926535897932384626433832795
 
void main()
{
	// Sample the surface color from the texture
  vec3 surface_color = texture(u_texture_color, vs_out_texcoord).rgb;
  
#define USE_NORMAL_MAP 1
#if USE_NORMAL_MAP 
  // Sample the normal from normal texture as color which is in range [0, 1].
  // Note: this vector is in tangent space.
  vec3 normal_sampled = texture(u_texture_normal, vs_out_texcoord).rgb;
	normal_sampled = normalize(normal_sampled*2.0 - 1.0); // now it's in range [-1, 1]
	// Transform the sampled normal vector from tangent space to world space
	vec3 n = normalize(vs_out_TBN * normal_sampled);
#else
	vec3 n = normalize(vs_out_normal_world_space);
#endif

  // Calculate the general formula for the irradiance due a point source, E:
  // E 	= (P / 4pi)(cos(theta) / r^2)
  // 		= I * (cos(theta) / r^2)
  // 
  // Where:
  // 
  // I = P / 4pi 
  // cos(theta) = n dot l
  
  float r = length(u_light_position - vs_out_frag_world_space); // r = ||p − x||
  vec3 l = normalize(u_light_position - vs_out_frag_world_space); // l = (p − x) / r 
  float angle_incidence = max(dot(n, l), 0.0);  // the cos(theta) term
  float I = u_light_power_watt / (4*PI);
  float E = I * (angle_incidence / (r*r));
  
  // Lambertian shading:
  // The very simplest kind of reflection is a surface that reflects light equally to all directions, 
  // regardless of where it came from, so that the reflected light `L_r` is simply a constant multiple of the irradiance:
  // 
  // Lr = kE
  //
  // The surface is described by its reflactance `R`, which is the fraction of the irradiance it reflects.
  // The coefficient relating reflected to incident light is R/pi :
  //
  // Lr = (R / pi)E
  // 
  // The reflectance can be different for different colors of light, and for simple modeling of color, 
  // it suffices to just keep three different reflectances, one each for red, green, and blue, 
  // so this shading equation is carried out separately for the three color channels.
  
  vec3 R = surface_color * u_light_color;
  vec3 Lr = (R / PI)*E;
  
  // Many materials have some degree of shininess to them, for examples: metals, plastics, gloss or semi-gloss paints.
  // Their color is view-dependent in contrast to the view-independent color of a Lambertian surface, and the view-dependent 
  // part of the reflection is known as specular reflection.
  // Many surfaces are not perfectly smooth, and they exhibit a more general kind of reflection known in as glossy reflection.
  // The simplest and well-known model is known as the Modified Blinn– Phong model.
  // it is a function of the view vector `𝑣` as well as the normal vector `n` and light direction `𝑙`.
  // The idea is to produce reflection that is at its brightest when `v` and `l` are symmetric across the surface normal,
  // We can tell how close we are to a mirror configuration using the idea of half vector `h` which is the vector halfway between 
  // `v` and `l`, and it is perpendicular to the surface exactly when `v` and `l` are in mirror reflection configuration.
  // If the vector $h$ is near to the surface normal then the specular is bright, otherwise it shoulf be dim.
  // We measure the nearness of `h` to `n` using the dot product and then take the result to a power `p` to make it decrease faster:
  // 
  // Lr = (n dot h)^p
  // 
  // Where the exponent `p` controls the shininess of the surface: higher values leads to shinier appearence.
  // The half vector $h$ is easy to compute:
  // 
  // h = normalize(l + v)
  // 
  // To incorporate the Blinn-Phong idea into the shading computation, we add the specular component to Lambertian Shading:
  // 
  // Lr = (R / pi)E + k_s * max(0, n dot h)^p * E
  // 		= (R / pi + k_s * max(0, n dot h)^p) E
  // 		= diffuse + specular
  
  vec3 v = normalize(u_camera_eye - vs_out_frag_world_space);
  vec3 h = normalize(l + v);
  vec3 ks = vec3(0.f); // Specular coefficient
  int p = 1024; // Shininess exponent
  float specular = pow(max(0.0, dot(n, h)), p);
  
  // Ambient illumination: it prevents shadows from being completely black and allows an easy way to tweak overall scene contrast.
  vec3 k_a = surface_color.rgb * vec3(0.15); 
  vec3 I_a = vec3(1.0);
  vec3 ambient = k_a * I_a;
  
  Lr = ambient + Lr + (ks * max(0, specular)*E);
  fs_out_color = vec4(Lr, 1.0);
}
