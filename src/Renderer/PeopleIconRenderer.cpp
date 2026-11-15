// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "PeopleIconRenderer.hpp"
#include "ui/canvas/Brush.hpp"
#include "ui/canvas/Canvas.hpp"
#include "ui/canvas/Pen.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace {

/** A person: a head above a body shaped like half an ellipse. */
struct Person {
  /* design units relative to the icon origin */
  double center_x, head_y, head_radius;
  double body_radius_x, body_top;
};

/* the icon is 37x31 design units */
constexpr double BODY_BOTTOM = 30.5;
constexpr Person BACK{25, 6.5, 5.5, 11, 14};
constexpr Person FRONT{12, 11, 6, 12.5, 19};

/** the front person gets an outline to separate it from the back one */
constexpr unsigned OUTLINE_WIDTH = 2;

constexpr unsigned BODY_STEPS = 12;

} // namespace

static void
DrawPerson(Canvas &canvas, const DesignGrid &grid, double x, double y,
           const Person &p) noexcept
{
  BulkPixelPoint body[BODY_STEPS + 1];
  const double body_height = BODY_BOTTOM - p.body_top;
  for (unsigned i = 0; i <= BODY_STEPS; ++i) {
    const double angle = std::numbers::pi * (1 + double(i) / BODY_STEPS);
    body[i] = {grid.X(x + p.center_x + p.body_radius_x * std::cos(angle)),
               grid.Y(y + BODY_BOTTOM + body_height * std::sin(angle))};
  }
  canvas.DrawPolygon(body, BODY_STEPS + 1);

  canvas.DrawCircle({grid.X(x + p.center_x), grid.Y(y + p.head_y)},
                    grid.Scale(p.head_radius));
}

void
DrawPeopleIcon(Canvas &canvas, const DesignGrid &grid, unsigned x, unsigned y,
               Color color) noexcept
{
  canvas.Select(Brush(color));

  canvas.SelectNullPen();
  DrawPerson(canvas, grid, x, y, BACK);

  canvas.Select(Pen(std::max(1, grid.Scale(OUTLINE_WIDTH)), COLOR_BLACK));
  DrawPerson(canvas, grid, x, y, FRONT);
}
