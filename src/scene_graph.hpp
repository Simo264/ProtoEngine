#pragma once

#include "static_mesh.hpp"
#include "transformation.hpp"

#include <glm/mat4x4.hpp>
#include <vector>
#include <memory>
#include <string>
#include <string_view>

// Scene graphs consist of a number of scene nodes, kept together in a tree-like structure - each node has a parent node, and a number of child nodes.
// Each node in the scene graph contains information relating to its graphical representation.
// For example: the chassis scene node could have a pointer to a car body mesh, while the four wheel nodes contain
// pointers to a single wheel mesh - there’s no need to load the same mesh in multiple times!
// But how does this solve how to keep track of where the wheels are in relation to the chassis? 
// Simple: scene graphs can also store the spatial relationship between a parent scene node and its children.
// 
// Each node in a scene graph contains data representing its local transform: its position and orientation in relation to its parent node.
// This is simply a transformation matrix. 
// Instead of translating a mesh from local space to world space, a scene node’s transformation matrix transforms its position locally in relation to its parent.
// This means that all transformation information cascades down the scene graph, including scaling transformations. 
// This means that all scene nodes that are children of a given node with a scale will have their transformation matrices scaled.
// 
// If we want to render the graph on screen we’ll need each node’s world transformation, to use as the model matrix in a vertex shader.
// In order to do this, we must traverse the graph Starting from the ’top’ or root of the scene graph.
// The world transformation for each node can be calculated by multiplying a node’s local transformation matrix with the world transformation of its parent.
// Generally, it is better to store both the ’local’ and ’world’ transformations of an object, by descending the graph from the root.
// If we didn’t store the world matrix, we’d have to traverse the tree every time we needed the world matrix of a node - in a deep graph that could be a lot of 
// unnecessary multiplications!
// 
// A scene node doesn’t necessarily have to contain graphical information, such as a mesh. 
// They may be purely transitional, that is, nodes that group together and translate a number of children, but don’t render anything themselves.

class SceneNode
{
public:
	SceneNode(std::string_view name);
	
	auto parent() const { return m_parent; }
	auto& children() const { return m_children; }
	auto mesh() const { return m_mesh; }
	auto& world_transform() const { return m_world_transformation; }
	auto& local_transform() const { return m_local_transformation; }
	
	void add_child(std::shared_ptr<SceneNode> child);
	
	// This method is recursive. Every time the transformation of the parent changes, we will recalculate all the child nodes.
	void set_parent(std::shared_ptr<SceneNode> parent);
	
	// If you change a node's local transform, its world_transform changes. But that of all his children, grandchildren and so on must also change!
	// This method is recursive. Every time the transformation of the parent changes, we will recalculate all the child nodes. 
	void set_transform(const Transformation& local_transf);
	
	void set_mesh(std::shared_ptr<StaticMesh> mesh) { this->m_mesh = mesh; }
	
	std::string name;
private:
	void update_world_transform();
	
	std::shared_ptr<SceneNode> m_parent;
	std::vector<std::shared_ptr<SceneNode>> m_children;
	std::shared_ptr<StaticMesh> m_mesh;
	Transformation m_local_transformation;
	glm::mat4 m_world_transformation;
};