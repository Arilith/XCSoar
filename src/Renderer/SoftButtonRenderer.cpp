// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "SoftButtonRenderer.hpp"
#include "GradientRenderer.hpp"
#include "Look/LXLook.hpp"
#include "Screen/Layout.hpp"
#include "ui/canvas/Canvas.hpp"
#include "ui/canvas/Font.hpp"
#include "ui/canvas/Pen.hpp"
#include "util/CharUtil.hxx"

#include <algorithm>
#include <cmath>
#include <string>

DesignGrid
SoftButtonRenderer::GetGrid() const noexcept
{
  if (fixed_grid)
    return *fixed_grid;

  const unsigned width = Layout::min_screen_pixels;
  return DesignGrid({width, width * LX::GRID_HEIGHT / LX::GRID_WIDTH},
                    {LX::GRID_WIDTH, LX::GRID_HEIGHT});
}

unsigned
SoftButtonRenderer::GetMinimumButtonWidth() const noexcept
{
  return GetGrid().Scale(LX::SoftKey::WIDTH);
}

PixelRect
SoftButtonRenderer::GetFaceRect(const PixelRect &rc,
                                const DesignGrid &grid) const noexcept
{
  const int width = grid.Scale(LX::SoftKey::WIDTH);
  const int height = grid.Scale(LX::SoftKey::HEIGHT);
  const int left = (rc.left + rc.right - width) / 2;
  return {left, rc.bottom - height, left + width, rc.bottom};
}

void
SoftButtonRenderer::DrawBadge(Canvas &canvas, const PixelRect &face,
                              const DesignGrid &grid) const noexcept
{
  const bool check = badge == Badge::CHECK;
  const LX::Badge::Palette &palette = check ? LX::Badge::GREEN
    : LX::Badge::RED;

  const int size = grid.Scale(LX::Badge::SIZE);
  const int left = (face.left + face.right - size) / 2;
  const int top = face.top - grid.Scale(LX::Badge::OVERHANG);
  const PixelRect rc{left, top, left + size, top + size};

  const unsigned radius = grid.Scale(LX::Badge::RADIUS);
  DrawGradientBox(canvas, rc, palette.fill, palette.fill, palette.edge,
                  radius);

  /* the lower edge is lighter than the others */
  const int inset = std::max(0, int(radius) - 1);
  canvas.DrawFilledRectangle({rc.left + inset, rc.bottom - 1,
                              rc.right - inset, rc.bottom},
                             palette.lower_edge);

  /* the glyph, given as fractions of the badge size */
  const auto at = [&](double x, double y) -> PixelPoint {
    return {left + int(std::lround(x * size)),
            top + int(std::lround(y * size))};
  };

  canvas.Select(Pen(std::max(1, grid.Scale(2)), LX::Badge::GLYPH));
  if (check) {
    canvas.DrawLine(at(0.25, 0.52), at(0.43, 0.70));
    canvas.DrawLine(at(0.43, 0.70), at(0.75, 0.30));
  } else {
    canvas.DrawLine(at(0.30, 0.30), at(0.70, 0.70));
    canvas.DrawLine(at(0.70, 0.30), at(0.30, 0.70));
  }
}

void
SoftButtonRenderer::DrawCaption(Canvas &canvas, const PixelRect &face,
                                ButtonState state,
                                const DesignGrid &grid) const noexcept
{
  /* LX shows soft keys in capital letters; converting ASCII only
     cannot break a multi-byte UTF-8 sequence */
  std::string text{caption.c_str()};
  for (char &ch : text)
    ch = ToUpperASCII(ch);

  canvas.Select(*font);
  canvas.SetBackgroundTransparent();
  canvas.SetTextColor(state == ButtonState::PRESSED ? COLOR_WHITE
                      : state == ButtonState::DISABLED
                      ? LX::SoftKey::DISABLED_TEXT : COLOR_BLACK);

  const int width = canvas.CalcTextSize(text.c_str()).width;
  const int baseline = face.top + grid.Scale(LX::SoftKey::BASELINE);
  canvas.DrawText({(face.left + face.right - width) / 2,
                   baseline - int(font->GetAscentHeight())},
                  text.c_str());
}

void
SoftButtonRenderer::DrawButton(Canvas &canvas, const PixelRect &rc,
                               ButtonState state) const noexcept
{
  const DesignGrid grid = GetGrid();
  const PixelRect face = GetFaceRect(rc, grid);

  if (state == ButtonState::FOCUSED || state == ButtonState::SELECTED) {
    PixelRect ring = face;
    ring.Grow(std::max(1, grid.Scale(LX::SoftKey::RING_OUTSET)));
    DrawGradientBox(canvas, ring, LX::SoftKey::FOCUS_RING,
                    LX::SoftKey::FOCUS_RING, LX::SoftKey::FOCUS_RING,
                    grid.Scale(LX::SoftKey::RADIUS));
  }

  const bool pressed = state == ButtonState::PRESSED;
  DrawGradientBox(canvas, face,
                  pressed ? LX::SoftKey::PRESSED : LX::SoftKey::TOP,
                  pressed ? LX::SoftKey::PRESSED : LX::SoftKey::BOTTOM,
                  LX::SoftKey::BORDER, grid.Scale(LX::SoftKey::RADIUS));

  DrawCaption(canvas, face, state, grid);

  if (badge != Badge::NONE)
    DrawBadge(canvas, face, grid);
}
