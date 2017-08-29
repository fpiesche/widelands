/*
 * Copyright (C) 2010-2017 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "graphic/game_renderer.h"

#include <memory>

#include "graphic/gl/coordinate_conversion.h"
#include "graphic/graphic.h"
#include "graphic/render_queue.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"
#include "wui/field_overlay_manager.h"
#include "wui/interactive_base.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"

void draw_border_markers(const FieldsToDraw::Field& field,
					  const float scale,
                 const FieldsToDraw& fields_to_draw,
                 RenderTarget* dst) {
	if (!field.all_neighbors_valid() || !field.is_border) {
		return;
	}
	assert(field.owner != nullptr);

	uint32_t const anim_idx = field.owner->tribe().frontier_animation();
	if (field.vision) {
		dst->blit_animation(
		   field.rendertarget_pixel, scale, anim_idx, 0, field.owner->get_playercolor());
	}
	for (const auto& nf : {fields_to_draw.at(field.rn_index), fields_to_draw.at(field.bln_index),
	                       fields_to_draw.at(field.brn_index)}) {
		if ((field.vision || nf.vision) && nf.is_border &&
		    (field.owner == nf.owner || nf.owner == nullptr)) {
			dst->blit_animation(middle(field.rendertarget_pixel, nf.rendertarget_pixel), scale,
			                    anim_idx, 0, field.owner->get_playercolor());
		}
	}
}

void draw_terrain(const Widelands::EditorGameBase& egbase,
                  const FieldsToDraw& fields_to_draw,
                  const float scale,
                  RenderTarget* dst) {
	const Recti& bounding_rect = dst->get_rect();
	const Surface& surface = dst->get_surface();
	const int surface_width = surface.width();
	const int surface_height = surface.height();

	// Enqueue the drawing of the terrain.
	RenderQueue::Item i;
	i.program_id = RenderQueue::Program::kTerrainBase;
	i.blend_mode = BlendMode::Copy;
	i.terrain_arguments.destination_rect =
	   Rectf(bounding_rect.x, surface_height - bounding_rect.y - bounding_rect.h, bounding_rect.w,
	         bounding_rect.h);
	i.terrain_arguments.gametime = egbase.get_gametime();
	i.terrain_arguments.renderbuffer_width = surface_width;
	i.terrain_arguments.renderbuffer_height = surface_height;
	i.terrain_arguments.terrains = &egbase.world().terrains();
	i.terrain_arguments.fields_to_draw = &fields_to_draw;
	i.terrain_arguments.scale = scale;
	RenderQueue::instance().enqueue(i);

	// Enqueue the drawing of the dither layer.
	i.program_id = RenderQueue::Program::kTerrainDither;
	i.blend_mode = BlendMode::UseAlpha;
	RenderQueue::instance().enqueue(i);

	// Enqueue the drawing of the road layer.
	i.program_id = RenderQueue::Program::kTerrainRoad;
	RenderQueue::instance().enqueue(i);
}
