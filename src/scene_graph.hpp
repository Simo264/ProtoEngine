#pragma once

#include "pipeline.hpp"
#include "static_mesh.hpp"
#include "transformation.hpp"
#include "lighthing.hpp"

#include <glm/ext/matrix_float4x4.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <optional>

// Scene graphs consist of a number of scene nodes, kept together in a tree-like
// structure - each node has a parent node, and a number of child nodes. Each
// node in the scene graph contains information relating to its graphical
// representation. For example: the chassis scene node could have a pointer to a
// car body mesh, while the four wheel nodes contain pointers to a single wheel
// mesh - there’s no need to load the same mesh in multiple times! But how does
// this solve how to keep track of where the wheels are in relation to the
// chassis? Simple: scene graphs can also store the spatial relationship between
// a parent scene node and its children.
//
// Each node in a scene graph contains data representing its local transform:
// its position and orientation in relation to its parent node. This is simply a
// transformation matrix. Instead of translating a mesh from local space to
// world space, a scene node's transformation matrix transforms its position
// locally in relation to its parent. This means that all transformation
// information cascades down the scene graph, including scaling transformations.
// As a result, all scene nodes that are children of a scaled node will have
// their transformation matrices affected by that scale.
//
// If we want to render the graph on screen we’ll need each node’s world
// transformation, to use as the model matrix in a vertex shader. To compute
// world transforms we typically traverse the graph starting from the root. The
// world transform for each node is computed by multiplying the node's local
// transform with the parent's world transform. For performance it's common to
// store both local and world transforms and update them during traversal so
// that repeated queries to a node's world transform don't require re-traversing
// the whole tree.
//
// A scene node doesn't necessarily have to contain graphical information, such
// as a mesh. They may be purely transitional nodes that group together and
// transform a number of children, but don't render anything themselves.
//
// API design note (parent/child relationships):
// - It's generally best to choose a single canonical way to establish
// parent/child relationships to avoid inconsistencies.
//   Common choices:
//     * make `add_child(child)` the canonical entry point: it sets the child's
//     parent and appends to the parent's children list.
//     * or make `set_parent(parent)` the canonical entry point: it removes the
//     node from its previous parent and appends to the new parent's children
//     list.
// - If you provide both `add_child` and `set_parent`, ensure one calls the
// other under-the-hood so both sides of the relation stay consistent.
// - `set_parent` can be convenient, but it often requires `shared_from_this()`
// (i.e. enable_shared_from_this) if the child needs to add itself to the
// parent's list.
//   That adds complexity; for that reason many APIs only expose
//   `add_child`/`remove_child` to users and keep `set_parent` internal or
//   carefully implemented.
//
// The implementation below currently exposes both `add_child` and `set_parent`.
// Consider making `set_parent` private or implementing it so that it always
// delegates to `parent->add_child(child)` (or vice-versa) to keep relationships
// consistent.
//
// ============================================================
// 
// Example usage of Scene + SceneNode:
//
// auto scene = Scene{};
// scene.set_root(scene.create_node("World"));
// 
// auto root_node = scene.root();
// auto car_node_1 = scene.create_node("Car_1");
// auto car_node_2 = scene.create_node("Car_2");
// root_node->add_child(car_node_1);
// car_node_1->add_child(car_node_2);
// car_node_1->set_transform(Transformation{ .position = { 0.0f, 0.0f, 0.0f }, .scale={ 0.05f,0.05f,0.05f } });
// car_node_2->set_transform(Transformation{ .position = { 0.0f, 0.0f, 100.0f } });
// car_node_1->set_mesh(car_mesh.get());
// car_node_2->set_mesh(car_mesh.get());
//
// while(...)
// {
// 	scene.update();
// 	render_scene(root);
// }
//  
// Note:
// - The Scene class owns all nodes (unique_ptr)
// - The SceneNode class contains only references to them (raw pointer)

class SceneNode
{
public:
  using Content = std::variant<std::monostate,
                               const StaticMesh*,
                               LightProperties>;

  SceneNode(std::string name) :
    name{ std::move(name) },
	  m_parent{ nullptr },
	  m_children{},
    m_content(std::monostate{}),
	  m_local_transformation{},
	  m_world_matrix{ 1.0f },
	  m_dirty{ false }
  {}

  auto parent() const { return m_parent; }
  auto& children() const { return m_children; }
 
  // Getter/Setter for mesh
  void set_mesh(const StaticMesh* mesh) { m_content = mesh; }
  auto has_mesh() const { return std::holds_alternative<const StaticMesh*>(m_content); }
  std::optional<const StaticMesh*> mesh();

  // Getter/Setter for light
  void set_light(const LightProperties& light) { m_content = light; }
  auto has_light() const { return std::holds_alternative<LightProperties>(m_content); }
  std::optional<LightProperties> light();
  auto world_light_position() const { return glm::vec3(m_world_matrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); }

  auto& world_matrix() const { return m_world_matrix; }
  auto& local_transform() const { return m_local_transformation; }

  void add_child(SceneNode* child);
  void set_transform(const Transformation &local_transf);
  void update_world();
  
  std::string name;

private:
	void mark_dirty();

  SceneNode* m_parent;
  std::vector<SceneNode*> m_children;
  
  Content m_content;

  Transformation m_local_transformation;
  glm::mat4 m_world_matrix;
  bool m_dirty; // dirty flag to indicate that the transformation of this node needs to be updated
};

class Scene
{
public:
	Scene() : m_nodes{}, m_root{ nullptr } {}
	
	SceneNode* create_node(std::string_view name);
	
	auto root() const { return m_root; }
	void set_root(SceneNode* root) { m_root = root; }
	
	void update(){ if(m_root) m_root->update_world(); }
	void render(class ShaderProgram program_vertex) const;
	
private:
	std::vector<std::unique_ptr<SceneNode>> m_nodes;
	SceneNode* m_root;
};