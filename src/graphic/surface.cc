/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/surface.h"

#include <cassert>
#include <cmath>
#include <cstdlib>

#include <SDL.h>

#include "base/macros.h"
#include "base/point.h"
#include "base/rect.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/utils.h"

namespace {

// Adjust 'original' so that only 'src_rect' is actually blitted.
BlitData adjust_for_src(BlitData blit_data, const Rect& src_rect) {
	blit_data.rect.x += src_rect.x;
	blit_data.rect.y += src_rect.y;
	blit_data.rect.w = src_rect.w;
	blit_data.rect.h = src_rect.h;
	return blit_data;
}

// Get the normal of the line between 'start' and 'end'.
template <typename PointType>
FloatPoint calculate_line_normal(const PointType& start, const PointType& end) {
	const float dx = end.x - start.x;
	const float dy = end.y - start.y;
	const float len = std::hypot(dx, dy);
	return FloatPoint(-dy / len, dx / len);
}

// Tesselates the line made up of 'points' ino triangles and converts them into
// OpenGL space for a renderbuffer of dimensions 'w' and 'h'.
// We need for each line four points. We do not make sure that these quads are
// not properly joined at the corners which does not seem to matter a lot for
// the thin lines that we draw in Widelands.
void tesselate_line_strip(int w,
								  int h,
								  const RGBColor& color,
								  float line_width,
								  const std::vector<FloatPoint>& points,
								  std::vector<DrawLineProgram::PerVertexData>* vertices) {
	const float r = color.r / 255.;
	const float g = color.g / 255.;
	const float b = color.b / 255.;

	// Iterate over each line segment, i.e. all points but the last, convert
	// them from pixel space to gl space and draw them.
	for (size_t i = 0; i < points.size() - 1; ++i) {
		const FloatPoint p1 = FloatPoint(points[i].x, points[i].y);
		const FloatPoint p2 = FloatPoint(points[i + 1].x, points[i + 1].y);

		const FloatPoint normal = calculate_line_normal(p1, p2);
		const FloatPoint scaled_normal(0.5f * line_width * normal.x, 0.5f * line_width * normal.y);

		// Quad points are created in rendering order for OpenGL.
		{
			FloatPoint p = p1 - scaled_normal;
			pixel_to_gl_renderbuffer(w, h, &p.x, &p.y);
			vertices->emplace_back(
			   DrawLineProgram::PerVertexData{p.x, p.y, 0.f, r, g, b, 1.});
		}

		{
			FloatPoint p = p2 - scaled_normal;
			pixel_to_gl_renderbuffer(w, h, &p.x, &p.y);
			vertices->emplace_back(
			   DrawLineProgram::PerVertexData{p.x, p.y, 0.f, r, g, b, 1.});
		}

		{
			FloatPoint p = p1 + scaled_normal;
			pixel_to_gl_renderbuffer(w, h, &p.x, &p.y);
			vertices->emplace_back(
			   DrawLineProgram::PerVertexData{p.x, p.y, 0.f, r, g, b, -1.});
		}

		vertices->push_back(vertices->at(vertices->size() - 2));
		vertices->push_back(vertices->at(vertices->size() - 2));

		{
			FloatPoint p = p2 + scaled_normal;
			pixel_to_gl_renderbuffer(w, h, &p.x, &p.y);
			vertices->emplace_back(
			   DrawLineProgram::PerVertexData{p.x, p.y, 0.f, r, g, b, -1.});
		}
	}
}

}  // namespace

void Surface::fill_rect(const Rect& rc, const RGBAColor& clr, BlendMode blend_mode) {
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), rc);
	do_fill_rect(rect, clr, blend_mode);
}

void Surface::brighten_rect(const Rect& rc, const int32_t factor)
{
	if (!factor) {
		return;
	}

	const BlendMode blend_mode = factor < 0 ? BlendMode::Subtract : BlendMode::UseAlpha;
	const int abs_factor = std::abs(factor);
	const RGBAColor color(abs_factor, abs_factor, abs_factor, 0);
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), rc);
	do_fill_rect(rect, color, blend_mode);
}

void Surface::draw_line_strip(std::vector<FloatPoint> points,
										const RGBColor& color,
										float line_width) {
	if (points.size() < 2) {
		return;
	}

	std::vector<DrawLineProgram::PerVertexData> vertices;
	// Each line needs 2 triangles.
	vertices.reserve(3 * 2 * points.size());
	tesselate_line_strip(width(), height(), color, line_width, points, &vertices);
	do_draw_line_strip(std::move(vertices));
}

void Surface::blit_monochrome(const Rect& dst_rect,
                              const Image& image,
                              const Rect& src_rect,
                              const RGBAColor& blend) {
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), dst_rect);
	do_blit_monochrome(rect, adjust_for_src(image.blit_data(), src_rect), blend);
}

void Surface::blit_blended(const Rect& dst_rect,
                           const Image& image,
                           const Image& texture_mask,
                           const Rect& src_rect,
                           const RGBColor& blend) {
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), dst_rect);
	do_blit_blended(rect, adjust_for_src(image.blit_data(), src_rect),
	                adjust_for_src(texture_mask.blit_data(), src_rect), blend);
}

void Surface::blit(const Rect& dst_rect,
                   const Image& image,
                   const Rect& src_rect,
                   float opacity,
                   BlendMode blend_mode) {
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), dst_rect);
	do_blit(rect, adjust_for_src(image.blit_data(), src_rect), opacity, blend_mode);
}

void draw_rect(const Rect& rc, const RGBColor& clr, Surface* surface) {
	const FloatPoint top_left = FloatPoint(rc.x + 0.5f, rc.y + 0.5f);
	const FloatPoint top_right = FloatPoint(rc.x + rc.w - 0.5f, rc.y + 0.5f);
	const FloatPoint bottom_right = FloatPoint(rc.x + rc.w - 0.5f, rc.y + rc.h - 0.5f);
	const FloatPoint bottom_left = FloatPoint(rc.x + 0.5f, rc.y + rc.h - 0.5f);

	surface->draw_line_strip({top_left, top_right, bottom_right}, clr, 1);
	// We need to split this up in order not to miss a pixel on the bottom right corner.
	surface->draw_line_strip({FloatPoint(bottom_right.x + 1, bottom_right.y), bottom_left, top_left}, clr, 1);
}
