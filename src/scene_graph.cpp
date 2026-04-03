#include "scene_graph.hpp"
#include "transformation.hpp"

#include <string_view>

// ===============================================
// 		SceneNode
// ===============================================

SceneNode::SceneNode(std::string_view name) : name{ name.data() }
{
	this->m_parent = nullptr;
	this->m_mesh = nullptr;
	this->m_children = {};
	this->m_local_transformation = Transformation{};
	this->m_world_matrix = glm::mat4{1.0f};
	this->m_dirty = false;
}

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

void SceneNode::update_world_transform()
{
	if(!m_dirty)
		return;
	
	m_local_transformation.update_tranformation();
	if(m_parent)
		m_world_matrix = m_parent->m_world_matrix * m_local_transformation.M;
	
	m_dirty = false;
	
	for(auto child : this->m_children)
		child->update_world_transform();
}

void SceneNode::mark_dirty()
{
	if (m_dirty) 
		return;

	m_dirty = true;
  for (auto child : m_children)
    child->mark_dirty();
}

// ===============================================
// 		Scene
// ===============================================

SceneNode* Scene::create_node(std::string_view name)
{
	m_nodes.emplace_back(std::make_unique<SceneNode>(name));
	return m_nodes.back().get();
}