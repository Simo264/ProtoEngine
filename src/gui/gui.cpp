#include "gui.hpp"

#include "camera.hpp"
#include "scene_graph.hpp"

#include <glm/vec3.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>

void gui_camera_window(Camera& camera)
{
	ImGui::Begin("Camera");
  auto fov = glm::degrees(camera.fovy);
  if (ImGui::DragFloat("Vertical FOV", &fov, 0.5f, 30.0f, 120.0f))
    camera.fovy = glm::radians(fov);

  auto euler_angles = glm::degrees(camera.get_euler_angles());
  ImGui::Text("Camera Position: X:%.2f, Y:%.2f, Z:%.2f", camera.eye.x, camera.eye.y, camera.eye.z);
	ImGui::BulletText("P: %.2f°  Y: %.2f°  R: %.2f°", euler_angles.x, euler_angles.y, euler_angles.z);
  if (ImGui::Button("Reset Camera")) 
  {
    camera.eye = glm::vec3(0.0f, 0.0f, 5.0f);
    camera.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    camera.fovy = glm::radians(45.0f);
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

  bool open = ImGui::TreeNodeEx( (void*)node,flags, "%s", node->name.data());
  if (ImGui::IsItemClicked())
    selected_node = node;

  if (open) 
  {
    for (auto child : node->children())
      draw_scene_node(child);

    ImGui::TreePop();
  }
}

void gui_hierarchy_window(SceneNode* node)
{
	ImGui::Begin("Scene graph");
	draw_scene_node(node);
	ImGui::End();
}

static void draw_inspector() 
{
  ImGui::Begin("Inspector");
  if (selected_node) 
  {
    auto t = selected_node->local_transform();
    auto changed = false;
    changed |= ImGui::DragFloat3("Position", &t.position.x, 0.1f);
    changed |= ImGui::DragFloat3("Rotation", &t.rotation.x, 0.5f);
    changed |= ImGui::DragFloat3("Scale", &t.scale.x, 0.1f);
    if (changed) 
    {
   		selected_node->set_transform(t);
    }
  }

  ImGui::End();
}
