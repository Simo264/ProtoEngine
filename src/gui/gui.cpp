#include "gui.hpp"

#include "../camera.hpp"
#include "../scene_graph.hpp"
#include "../transformation.hpp"
#include "../static_mesh.hpp"

#include <print>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/quaternion.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

extern f32 aspect_ratio;

GLFWwindow* init_glfw_context(i32 width, i32 height, std::string_view title)
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  auto window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);
  glfwSetFramebufferSizeCallback(window, []([[maybe_unused]] GLFWwindow *window, i32 width, i32 height) {
    glViewport(0, 0, width, height);
    aspect_ratio = static_cast<f32>(width) / static_cast<f32>(height);
  });
  glViewport(0, 0, width, height);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;           // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
  io.Fonts->AddFontFromFileTTF("fonts/Iceland-Regular.ttf", 16.f);
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");
  return window;
}

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
  auto transform_changed = false;
  transform_changed |= ImGui::DragFloat3("Position", &t.position.x, 0.01f);
  transform_changed |= ImGui::DragFloat3("Rotation", &t.rotation.x, 0.01f);
  transform_changed |= ImGui::DragFloat3("Scale", &t.scale.x, 0.01f);
  if (transform_changed)
    selected_node->set_transform(t);

  // Mesh section
  // =================================
  if (auto opt = selected_node->mesh_instance())
  {
    ImGui::SeparatorText("Mesh");
    auto& mesh_inst = opt.value().get();
    ImGui::Text("Vertices: %u", mesh_inst.mesh->nr_vertices());
    ImGui::Text("Indices: %u", mesh_inst.mesh->nr_indices());

    auto& material = mesh_inst.material;
    ImGui::ColorEdit3("Albedo", &material.albedo_color[0]);
    auto bool_metallic = material.metallic_factor > 0.0f;
    if (ImGui::Checkbox("Metallic", &bool_metallic))
      material.metallic_factor = bool_metallic ? 1.0f : 0.0f;
    ImGui::DragFloat("Roughness", &material.roughness_factor, 0.01f, 0.0f, 1.0f);
  }
  // Light section
  // =================================
  else if (auto opt = selected_node->light_instance()) 
  {
    ImGui::SeparatorText("Light");
    // Prendi una copia modificabile
    auto& light_inst = opt.value().get();
    ImGui::ColorEdit3("Color", &light_inst.color.x);
    ImGui::DragFloat("Power", &light_inst.power, 0.1f, 0.0f, 100.0f);
  }
  else 
  {
    ImGui::SeparatorText("Content");
    ImGui::Text("(empty node)");
  }

  ImGui::End();
}

// =======================================

void start_imgui_frame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void imgui_render()
{
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  auto& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) 
  {
    auto backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}