// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "GradientRenderer.hpp"
#include "ui/canvas/Canvas.hpp"

#include <algorithm>

#if defined(EYE_CANDY) && defined(ENABLE_OPENGL)

#include "ui/canvas/opengl/VertexPointer.hpp"
#include "util/Macros.hpp"

#endif

void
DrawVerticalGradient([[maybe_unused]] Canvas &canvas, const PixelRect &rc,
                     [[maybe_unused]] Color top_color, [[maybe_unused]] Color bottom_color, [[maybe_unused]] Color fallback_color)
{
#if defined(EYE_CANDY) && defined(ENABLE_OPENGL)
  const BulkPixelPoint vertices[] = {
    rc.GetTopLeft(),
    rc.GetTopRight(),
    rc.GetBottomLeft(),
    rc.GetBottomRight(),
  };

  const ScopeVertexPointer vp(vertices);

  const Color colors[] = {
    top_color,
    top_color,
    bottom_color,
    bottom_color,
  };

  const ScopeColorPointer cp(colors);

  static_assert(ARRAY_SIZE(vertices) == ARRAY_SIZE(colors),
                "Array size mismatch");

  glDrawArrays(GL_TRIANGLE_STRIP, 0, ARRAY_SIZE(vertices));
#else
  canvas.DrawFilledRectangle(rc, fallback_color);
#endif
}

void
DrawBandedVerticalGradient(Canvas &canvas, const PixelRect &rc,
                           Color top_color, Color bottom_color)
{
  const int height = rc.GetHeight();
  if (height <= 0)
    return;

  constexpr unsigned N_BANDS = 64;
  for (unsigned i = 0; i < N_BANDS; i++) {
    const int y0 = rc.top + (int)((unsigned)height * i / N_BANDS);
    const int y1 = rc.top + (int)((unsigned)height * (i + 1) / N_BANDS);
#ifdef GREYSCALE
    const uint8_t l = top_color.GetLuminosity()
      + (int(bottom_color.GetLuminosity()) - int(top_color.GetLuminosity()))
        * (int)i / (int)(N_BANDS - 1);
    canvas.DrawFilledRectangle(PixelRect{PixelPoint{rc.left, y0},
                                         PixelPoint{rc.right, y1}},
                               Color(l));
#else
    const uint8_t r = top_color.Red()
      + (int(bottom_color.Red()) - int(top_color.Red()))
        * (int)i / (int)(N_BANDS - 1);
    const uint8_t g = top_color.Green()
      + (int(bottom_color.Green()) - int(top_color.Green()))
        * (int)i / (int)(N_BANDS - 1);
    const uint8_t b = top_color.Blue()
      + (int(bottom_color.Blue()) - int(top_color.Blue()))
        * (int)i / (int)(N_BANDS - 1);
    canvas.DrawFilledRectangle(PixelRect{PixelPoint{rc.left, y0},
                                         PixelPoint{rc.right, y1}},
                               Color(r, g, b));
#endif
  }
}

static constexpr uint8_t
Interpolate(uint8_t a, uint8_t b, unsigned i, unsigned n) noexcept
{
  return uint8_t((a * (n - i) + b * i + n / 2) / n);
}

/** The colour at step i of n between two colours. */
static constexpr Color
Interpolate(Color a, Color b, unsigned i, unsigned n) noexcept
{
#ifdef GREYSCALE
  return Color(Interpolate(a.GetLuminosity(), b.GetLuminosity(), i, n));
#else
  return Color(Interpolate(a.Red(), b.Red(), i, n),
               Interpolate(a.Green(), b.Green(), i, n),
               Interpolate(a.Blue(), b.Blue(), i, n));
#endif
}

void
DrawGradientBox(Canvas &canvas, const PixelRect &rc,
                Color top_color, Color bottom_color, Color border_color,
                unsigned radius) noexcept
{
  const int height = rc.GetHeight();
  if (height < 2 || rc.GetWidth() < 2)
    return;

  for (int i = 0; i < height; ++i) {
    /* distance from the nearest horizontal edge; the corner pixels
       are missing in the outermost rows */
    const int edge_distance = std::min(i, height - 1 - i);
    const int inset = std::max(0, int(radius) - 1 - edge_distance);
    const int left = rc.left + inset, right = rc.right - inset;
    const int y = rc.top + i;

    if (edge_distance == 0) {
      canvas.DrawFilledRectangle({left, y, right, y + 1}, border_color);
      continue;
    }

    const Color color = Interpolate(top_color, bottom_color, i, height - 1);

    canvas.DrawFilledRectangle({left, y, right, y + 1}, color);
    canvas.DrawFilledRectangle({left, y, left + 1, y + 1}, border_color);
    canvas.DrawFilledRectangle({right - 1, y, right, y + 1}, border_color);
  }
}
