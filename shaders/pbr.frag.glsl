#version 460 core

in vec2 vs_out_texcoord;
in vec3 vs_out_normal_world_space;
in vec3 vs_out_frag_world_space;
in mat3 vs_out_TBN;
out vec4 fs_out_color;

layout(location = 0) uniform vec3 u_camera_eye;
layout(location = 1) uniform int  u_has_texture_albedo;
layout(location = 2) uniform int  u_has_texture_normal;

layout(location = 5) uniform vec3 u_albedo;
layout(location = 6) uniform float u_metallic;
layout(location = 7) uniform float u_roughness;

layout(location = 10) uniform vec3 u_light_position;
layout(location = 11) uniform vec3 u_light_color;
layout(location = 12) uniform float u_light_power_watt;

// texture bindings
layout(binding = 0) uniform sampler2D u_texture_albedo;
layout(binding = 1) uniform sampler2D u_texture_normal;
layout(binding = 2) uniform sampler2D u_texture_roughness;

#define PI 3.1415926535897932384626433832795

vec3 fresnel_schlick(float cos_theta, vec3 F0) 
{
  return F0 + (1.0f - F0) * pow(clamp(1.0f - cos_theta, 0.0f, 1.0f), 5.0f);
}

float distribution_ggx(vec3 n, vec3 h, float roughness) 
{
  float a = roughness * roughness; // alpha remapping (perceptually more linear)
  float a2 = a * a;
  float NdotH = max(dot(n, h), 0.0f);
  float NdotH2 = NdotH * NdotH;

  float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
  denom = PI * denom * denom;

  return a2 / max(denom, 1e-6);
}

float geometry_schlick_ggx(float NdotX, float k) 
{
  float denom = NdotX * (1.0f - k) + k;
  return NdotX / max(denom, 1e-6);
}

float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness) 
{
  float NdotV = max(dot(n, v), 0.0);
  float NdotL = max(dot(n, l), 0.0);

  // k for direct lighting (different from the k used for IBL)
  float r = roughness + 1.0f;
  float k = (r * r) / 8.0f;

  float ggx_v = geometry_schlick_ggx(NdotV, k); // G1(v)
  float ggx_l = geometry_schlick_ggx(NdotL, k); // G1(l)

  return ggx_v * ggx_l; // G2 = G1(v) * G1(l)
}

vec3 brdf_specular(vec3 n, vec3 v, vec3 l, vec3 h, vec3 F, float roughness)
{
  float D = distribution_ggx(n, h, roughness); // D(h)
  float G = geometry_smith(n, v, l, roughness); // G2(v, l)

  float NdotV = max(dot(n, v), 0.0f);
  float NdotL = max(dot(n, l), 0.0f);

  vec3 numerator = F * G * D;
  float denominator = 4.0f * NdotV * NdotL;
  
  return numerator / max(denominator, 1e-6);
}

vec3 brdf_diffuse(vec3 albedo, float metallic, vec3 F)
{
  vec3 kS = F;                                    // energy going into specular
  vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);  // energy left for diffuse
                                                  // metals have (almost) no diffuse term
  return kD * albedo / PI; // lambertian diffuse
}

vec3 brdf(vec3 n, vec3 v, vec3 l, vec3 h, vec3 albedo, float metallic, float roughness)
{
  // F0: base reflectance at normal incidence
  // dielectrics ~0.04 (achromatic), metals use albedo as F0 (colored)
  vec3 F0 = mix(vec3(0.04f), albedo, metallic);

  // Fresnel evaluated once, angle between H and V (= angle between H and L)
  vec3 F = fresnel_schlick(max(dot(h, v), 0.0), F0);
  
  vec3 spec = brdf_specular(n, v, l, h, F, roughness);
  vec3 diff = brdf_diffuse(albedo, metallic, F);
  return spec + diff;
}

vec3 linear_to_srgb(vec3 color, float gamma) 
{
  return pow(color, vec3(1.0f / gamma));
}

void main()
{
  // ============================================================
  // The REFLECTANCE EQUATION
  // ============================================================
  //
  // Lo(p, v) = brdf(l, v) * Li(p, l) * (n dot l)
  //
  // where:
  //   Lo(p, v): outgoing radiance at point p, in direction v (towards the camera)
  //   Li(p, l): incident radiance arriving at point p, from direction l (towards the light)
  //   brdf(l, v): the BRDF that describes how much incoming light from l is reflected towards v
  //
  // ------------------------------------------------------------
  // Li(p, l): incident radiance from the point light
  // ------------------------------------------------------------
  // For a physically-based point light:
  //
  //   I = phi / (4pi)        -> radiant intensity (W/sr), assuming isotropic emission
  //   E = I / d^2            -> irradiance at distance d (inverse square law)
  //   Li = light_color * E   -> incident "radiance" used in the discrete sum
  //
  // ------------------------------------------------------------
  // The BRDF (specular term via microfacet theory)
  // ------------------------------------------------------------
  //
  //                    F(h,v) * G2(l,v,h) * D(h)
  // brdf_spec(l,v) = ──────────────────────────
  //                   4 * |n dot l| * |n dot v|
  //
  //   F  : Fresnel reflectance (Schlick approximation)
  //   D  : Normal Distribution Function (GGX/Trowbridge-Reitz)
  //   G2 : joint masking-shadowing function (Smith, G2 = G1(l) * G1(v))
  //
  // brdf(l,v) = brdf_diffuse(l,v) + brdf_spec(l,v)

  
  // definition of light properties
  float radiant_intensity = u_light_power_watt / (4.0f * PI);
  float light_distance = distance(u_light_position, vs_out_frag_world_space);
  float irradiance = radiant_intensity / (light_distance * light_distance);
  vec3 incident_energy = u_light_color * irradiance;
  vec3 Li = incident_energy; // incident energy at the light source
  
  vec3 n = normalize(vs_out_normal_world_space);
  vec3 v = normalize(u_camera_eye - vs_out_frag_world_space);
  vec3 l = normalize(u_light_position - vs_out_frag_world_space);
  vec3 h = normalize(l + v);

  float ndotl = max(dot(n, l), 0.0f);

  vec3 Lo = brdf(n, v, l, h, u_albedo, u_metallic, u_roughness) * Li * ndotl;

  vec3 ambient = u_albedo * 0.01f;
  
  // HDR tone mapping
  vec3 color = ambient + Lo;
  color = color / (color + vec3(1.0));
  // sRGB encoding
  color = linear_to_srgb(color, 2.2f);
  fs_out_color = vec4(color, 1.0f);
}