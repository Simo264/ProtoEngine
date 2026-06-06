#include "scene_graph.hpp"
#include "transformation.hpp"
#include "pipeline.hpp"

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

std::optional<const StaticMesh*> SceneNode::mesh()
{ 
  try 
  { 
    return std::get<const StaticMesh*>(m_content); 
  }
  catch(const std::bad_variant_access& e) 
  { 
    return std::nullopt; 
  }
}

std::optional<LightProperties> SceneNode::light()
{ 
  try 
  { 
    return std::get<LightProperties>(m_content); 
  }
  catch(const std::bad_variant_access& e) 
  { 
    return std::nullopt; 
  }
}

// ===============================================
// 		Scene
// ===============================================

SceneNode* Scene::create_node(std::string_view name)
{
	m_nodes.emplace_back(std::make_unique<SceneNode>(name.data()));
	return m_nodes.back().get();
}

void Scene::render(ShaderProgram program_vertex) const
{
 if(!m_root)
   return;
 
  auto node_stack = std::stack<SceneNode*>{};
  node_stack.push(m_root);
  while (!node_stack.empty())
  {
    auto current = node_stack.top();
    node_stack.pop();
    
    auto opt = current->mesh();
    if (opt)
    {
      auto mesh = opt.value();

      auto& mat_transform = current->world_matrix();
      program_vertex.set_uniform_mat4f(0, mat_transform); // mat_transform => location 0
      
      mesh->vao().bind();
      glDrawElements(GL_TRIANGLES, mesh->nr_indices(), GL_UNSIGNED_INT, 0);
    }
    
    auto& children = current->children();
    for (auto it = children.rbegin(); it != children.rend(); ++it)
    {
      if (*it)
        node_stack.push(*it);
    }
  }
}