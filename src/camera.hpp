#pragma once

#include "basic_types.hpp"

#include <glm/mat4x4.hpp>

void init_camera(f32 near, f32 far, f32 fovy, f32 aspect_ratio);

void frame_to_canonical(f32* vertices, i32 n_vertices);
void canonical_to_camera(f32* vertices, i32 n_vertices);
void camera_to_perspective(f32* vertices, i32 n_vertices);
void camera_to_ortho(f32* vertices, i32 n_vertices);

const glm::mat4& get_mat_ftc();
const glm::mat4& get_mat_camera();
const glm::mat4& get_mat_perspective();
const glm::mat4& get_mat_ortho();
