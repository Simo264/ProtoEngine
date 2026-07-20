# Roadmap 

##  Illuminazione e materiali (PBR core)
- [ ] Transizione da Blinn-Phong a Cook-Torrance BRDF
- [ ] Normal Distribution Function (GGX / Trowbridge-Reitz)
- [ ] Termine geometrico (Smith, Schlick-GGX)
- [ ] Fresnel (Schlick approximation)
- [ ] Sistema materiali metallic/roughness (albedo, metallic, roughness, normal, AO)
- [ ] Normal mapping + tangent space (calcolo TBN matrix)
- [ ] Multi-light system (directional, point, spot) con attenuazione fisica

## Color management
- [ ] Rendering in linear space
- [ ] Gamma correction (sRGB conversion in output)
- [ ] Framebuffer HDR (floating point)
- [ ] Tone mapping (Reinhard / ACES filmic / Uncharted 2)
- [ ] Camera fisicamente basata (esposizione, EV)

## Image-Based Lighting (IBL)
- [ ] Caricamento HDR environment map
- [ ] Irradiance map (convoluzione diffusa)
- [ ] Prefiltered environment map (specular, mip levels per roughness)
- [ ] BRDF LUT (split-sum approximation)
- [ ] Integrazione IBL come ambient lighting

## Ombre
- [ ] Shadow mapping base (directional light)
- [ ] PCF (Percentage Closer Filtering)
- [ ] Cascaded Shadow Maps (CSM)
- [ ] Point light shadows (cubemap)
- [ ] Spot light shadows
- [ ] (Opzionale) Variance / Exponential Shadow Maps

## Architettura del rendering
- [ ] Forward rendering (baseline)
- [ ] Deferred rendering (G-buffer)
- [ ] Forward+ / Clustered shading
- [ ] Frustum culling
- [ ] Occlusion culling
- [ ] Instancing
- [ ] LOD (Level of Detail)

## Post processing
- [ ] Bloom
- [ ] SSAO
- [ ] Anti-aliasing (MSAA / FXAA / TAA)
- [ ] Depth of field
- [ ] Motion blur
- [ ] Screen Space Reflections (SSR)

## Trasparenza
- [ ] Alpha blending ordinato
- [ ] Order-Independent Transparency (OIT)
- [ ] Alpha testing (foliage, masked materials)

## Tooling
- [ ] Debug visualization (normali, wireframe, depth, luci)
- [ ] GPU profiling (timer query, frame breakdown)
- [ ] Serializzazione scena/materiali

## Argomenti avanzati
- [ ] Global Illumination real-time (voxel cone tracing / light probes / SSGI)
- [ ] Skeletal animation + skinning
- [ ] Particle systems
- [ ] Compute shader (culling, clustered lighting, simulazioni GPU-driven)