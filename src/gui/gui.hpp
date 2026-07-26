#pragma once

#include "../basic_types.hpp"
#include <glm/ext/vector_float3.hpp>
#include <string_view>

struct GLFWwindow* init_glfw_context(i32 width, i32 height, std::string_view title);

void gui_camera(class Camera& camera);
void gui_hierarchy(class SceneNode* node);
void gui_inspector(glm::vec3& albedo, f32& metallic, f32& roughness);

void start_imgui_frame();
void imgui_render();