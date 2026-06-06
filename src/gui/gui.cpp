#include "gui.hpp"

#include "camera.hpp"
#include "scene_graph.hpp"
#include "transformation.hpp"

#include <print>

#include <glm/vec3.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>

void gui_camera(Camera& camera)
{
	ImGui::Begin("Camera");
  
  // FOV, aspect, near, far
  // =================================
  ImGui::Text("Projection");
  ImGui::Separator();
  auto fov = glm::degrees(camera.fovy);
  if (ImGui::DragFloat("Vertical FOV", &fov, 0.5f, 30.0f, 120.0f, "%.1f°"))
    camera.fovy = glm::radians(fov);

  ImGui::DragFloat("Aspect Ratio", &camera.aspect, 0.05f, 0.1f, 5.0f, "%.2f");
  ImGui::DragFloat("Near Plane", &camera.near, 0.01f, 0.001f, 10.0f, "%.3f");
  ImGui::DragFloat("Far Plane",  &camera.far,  1.0f,  10.0f,  2000.0f, "%.1f");
  ImGui::Spacing();

  // Position and orientation
  // =================================
  ImGui::Text("Position and orientation");
  ImGui::Separator();

  ImGui::DragFloat3("Position", &camera.eye.x, 0.05f, -100.0f, 100.0f, "%.2f");
  auto euler_angles = glm::degrees(camera.get_euler_angles());
  ImGui::BulletText("Pitch: %.2f°  Yaw: %.2f°  Roll: %.2f°", euler_angles.x, euler_angles.y, euler_angles.z);
  ImGui::Spacing();
  ImGui::Separator();
  
  // Reset camera
  // =================================
  if (ImGui::Button("Reset Camera")) 
  {
    camera.eye = glm::vec3(0.0f, 0.0f, 3.0f);
    camera.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    camera.fovy = glm::radians(45.0f);
    camera.near = 0.1f;
    camera.far = 100.0f;
  }

  ImGui::End();
}

// =======================================

static SceneNode* selected_node = nullptr;
static void draw_scene_node(SceneNode* node)
{
  auto flags = node->children().empty() ? ImGuiTreeNodeFlags_Leaf : 0;
  if (node == selected_node)
    flags |= ImGuiTreeNodeFlags_Selected;

  bool open = ImGui::TreeNodeEx( (void*)node, flags, "%s", node->name.data());
  if (ImGui::IsItemClicked())
    selected_node = node;

  if (open) 
  {
    for (auto child : node->children())
      draw_scene_node(child);

    ImGui::TreePop();
  }
}

void gui_hierarchy(SceneNode* node)
{
  if(!node)
  {
    std::println("Invalid scene node!");
    return;
  }
  
	ImGui::Begin("Scene graph");
  draw_scene_node(node);
	ImGui::End();
}

void gui_inspector() 
{
  ImGui::Begin("Inspector");
  if (!selected_node) 
  {
    ImGui::Text("No node selected");
    ImGui::End();
    return;
  }

  // Transform section
  // ====================================
  ImGui::SeparatorText("Transform");
  auto t = selected_node->local_transform();
  bool transform_changed = false;
  transform_changed |= ImGui::DragFloat3("Position", &t.position.x, 0.01f);
  transform_changed |= ImGui::DragFloat3("Rotation", &t.rotation.x, 0.01f);
  transform_changed |= ImGui::DragFloat3("Scale", &t.scale.x, 0.01f);
  if (transform_changed)
    selected_node->set_transform(t);

  // Mesh section
  // =================================
  if (selected_node->has_mesh()) 
  {
    ImGui::SeparatorText("Mesh");
    auto* mesh = selected_node->mesh().value();
    ImGui::Text("Vertices: %u", mesh->nr_vertices());
    ImGui::Text("Indices: %u", mesh->nr_indices());
  }
  // Light section
  // =================================
  else if (selected_node->has_light()) 
  {
    ImGui::SeparatorText("Light");
    // Prendi una copia modificabile
    auto light_props = selected_node->light().value();
    auto light_changed = false;
    light_changed |= ImGui::ColorEdit3("Color", &light_props.color.x);
    light_changed |= ImGui::DragFloat("Power", &light_props.power, 0.1f, 0.0f, 100.0f);
    if (light_changed)
      selected_node->set_light(light_props);
  }
  else 
  {
    ImGui::SeparatorText("Content");
    ImGui::Text("(empty node)");
  }

  ImGui::End();
}
