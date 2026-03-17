#pragma once

#include "basic_types.hpp"

void init_camera(f32 aspect_ratio);

void frame_to_canonical(f32* vertices, i32 n_vertices);
void canonical_to_camera(f32* vertices, i32 n_vertices);
void camera_to_perspective(f32* vertices, i32 n_vertices);
void camera_to_ortho(f32* vertices, i32 n_vertices);