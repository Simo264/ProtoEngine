#include "scene_graph.hpp"
#include "render_types.hpp"
#include "transformation.hpp"
#include "pipeline.hpp"
#include "static_mesh.hpp"

#include <glad/gl.h>

#include <string_view>
#include <stack>

// ===============================================
// 		SceneNode
// ===============================================

void SceneNode::add_child(SceneNode* child) 
{
 	child->m_parent = this;
	this->m_children.push_back(child);
}

void SceneNode::set_transform(const Transformation& local_transf)
{
	this->m_local_transformation = local_transf;
	mark_dirty();
}

void SceneNode::update_world()
{
	if (m_dirty) 
	{
    m_local_transformation.update_tranformation();
    m_dirty = false;
  }
	
	if(m_parent)
		m_world_matrix = m_parent->m_world_matrix * m_local_transformation.M;
	else
	  m_world_matrix = m_local_transformation.M;
	  
	for(auto child : this->m_children)
		child->update_world();
}

void SceneNode::mark_dirty()
{
	m_dirty = true;
  for (auto child : m_children)
    child->mark_dirty();
}

std::optional<std::reference_wrapper<MeshInstance>> SceneNode::mesh_instance()
{
  if (auto instance = std::get_if<MeshInstance>(&m_content)) 
    return std::ref(*instance);

  return std::nullopt;
}

std::optional<std::reference_wrapper<LightInstance>> SceneNode::light_instance()
{
  if (auto* instance = std::get_if<LightInstance>(&m_content)) 
    return std::ref(*instance);

  return std::nullopt;
}

// ===============================================
// 		Scene
// ===============================================

SceneNode* Scene::create_node(std::string_view name)
{
	m_nodes.emplace_back(std::make_unique<SceneNode>(name.data()));
	return m_nodes.back().get();
}

void Scene::render(ProgramPipelineObject pipeline,
                   ShaderProgram program_vertex,
                   ShaderProgram program_fragment) const
{
  if(!m_root)
    return;

  auto node_stack = std::stack<SceneNode*>{};
  node_stack.push(m_root);
  while (!node_stack.empty())
  {
    auto current = node_stack.top();
    node_stack.pop();
    
    // setup lights
    if (auto opt_light = current->light_instance())
      setup_light_node(current, opt_light.value(), pipeline, program_fragment);
    
    // render meshes
    else if (auto opt_mesh = current->mesh_instance())
      render_mesh_node(current, opt_mesh.value(), pipeline, program_vertex, program_fragment);

    auto& children = current->children();
    for (auto it = children.rbegin(); it != children.rend(); ++it)
    {
      if (*it)
        node_stack.push(*it);
    }
  }
}


void Scene::setup_light_node(const SceneNode* node, 
                             const LightInstance& light,
                             ProgramPipelineObject pipeline, 
                             ShaderProgram fragment_program) const 
{
  pipeline.set_active_program(fragment_program);
  fragment_program.set_uniform_vector3f(ShaderLocation::Fragment::LightPosition, node->world_light_position());
  fragment_program.set_uniform_vector3f(ShaderLocation::Fragment::LightColor, light.color);
  fragment_program.set_uniform_f32(ShaderLocation::Fragment::LightPowerWatt, light.power);
}

void Scene::render_mesh_node(const SceneNode* node, 
                             const MeshInstance& instance,
                             ProgramPipelineObject pipeline, 
                             ShaderProgram vertex_program,
                             ShaderProgram fragment_program) const 
{
  auto* mesh = instance.mesh;
  if (!mesh) 
    return;

  pipeline.set_active_program(vertex_program);
  vertex_program.set_uniform_mat4f(ShaderLocation::Vertex::MatTransform, node->world_matrix());

  pipeline.set_active_program(fragment_program);
  auto& material = instance.material;
  auto metallic = material.metallic_factor;
  auto roughness = material.roughness_factor;

  // albedo color
  auto has_texture_albedo = material.tex_albedo.is_valid();
  fragment_program.set_uniform_vector3f(ShaderLocation::Fragment::Albedo, material.albedo_color);
  fragment_program.set_uniform_i32(ShaderLocation::Fragment::HasTextureAlbedo, static_cast<i32>(has_texture_albedo));
  if (has_texture_albedo)
    material.tex_albedo.bind_texture_unit(ShaderLocation::Texture::AlbedoUnit);

  // metallic
  auto has_texture_metallic = material.tex_metallic.is_valid();
  fragment_program.set_uniform_f32(ShaderLocation::Fragment::Metallic, metallic);
  fragment_program.set_uniform_i32(ShaderLocation::Fragment::HasTextureMetallic, static_cast<i32>(has_texture_metallic));
  if (has_texture_metallic)
    material.tex_metallic.bind_texture_unit(ShaderLocation::Texture::MetallicUnit);

  // roughness
  auto has_texture_roughness = material.tex_roughness.is_valid();
  fragment_program.set_uniform_f32(ShaderLocation::Fragment::Roughness, roughness);
  fragment_program.set_uniform_i32(ShaderLocation::Fragment::HasTextureRoughness, static_cast<i32>(has_texture_roughness));
  if (has_texture_roughness)
    material.tex_roughness.bind_texture_unit(ShaderLocation::Texture::RoughnessUnit);

  // normal map
  auto has_texture_normal = material.tex_normal.is_valid();
  fragment_program.set_uniform_i32(ShaderLocation::Fragment::HasTextureNormal, static_cast<i32>(has_texture_normal));
  if (has_texture_normal) 
    material.tex_normal.bind_texture_unit(ShaderLocation::Texture::NormalUnit);

  mesh->vao().bind();
  glDrawElements(GL_TRIANGLES, mesh->nr_indices(), GL_UNSIGNED_INT, 0);
}