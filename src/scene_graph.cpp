#include "scene_graph.hpp"
#include "transformation.hpp"

#include <string_view>

// ===============================================
// 		PUBLIC
// ===============================================

SceneNode::SceneNode(std::string_view name) : name{ name.data() }
{
	this->m_parent = nullptr;
	this->m_mesh = nullptr;
	this->m_children = {};
	this->m_local_transformation = Transformation{};
	this->m_world_transformation = glm::mat4{1.0f};
}

void SceneNode::add_child(std::shared_ptr<SceneNode> child)
{
	this->m_children.push_back(child);
}

void SceneNode::set_parent(std::shared_ptr<SceneNode> parent)
{
	this->m_parent = parent;
	// update the world transformation of this mesh and all of its children
	this->update_world_transform();
}

void SceneNode::set_transform(const Transformation& local_transf)
{
	this->m_local_transformation = local_transf;
	// update the world transformation of this mesh and all of its children
	this->update_world_transform();
}

// ===============================================
// 		PRIVATE
// ===============================================

void SceneNode::update_world_transform()
{
	m_local_transformation.update_tranformation();
	
	// this->world_transformation = parent.world_transformation * this->local_transformation;
	if(m_parent)
		m_world_transformation = m_parent->m_world_transformation * m_local_transformation.M;
	else 
		m_world_transformation = m_local_transformation.M;
	
	for(auto child : this->m_children)
		child->update_world_transform();
}