#include "scene_graph.hpp"
#include "transformation.hpp"

SceneNode::SceneNode()
{
	this->m_parent = nullptr;
	this->m_mesh = nullptr;
	this->m_children = {};
	this->m_local_transformation = Transformation{};
	this->m_world_transformation = Transformation{};
}

void SceneNode::add_child(std::shared_ptr<SceneNode> child)
{
	this->m_children.push_back(child);
}

void SceneNode::update_world_transform()
{
	if(m_parent)
		m_world_transformation = m_parent->world_transform().calculate_tranformation() * m_local_transformation.calculate_tranformation();
	else 
		m_world_transformation = m_local_transformation;

}

void SceneNode::set_parent(std::shared_ptr<SceneNode> parent)
{
	this->m_parent = parent;
	// update the world transformation
	// this->m_world_transformation = ...;
}

void SceneNode::set_transform(const Transformation& local_transf)
{
	this->m_local_transformation = local_transf;
	// update the world transformation
	// this->m_world_transformation = ...;
}