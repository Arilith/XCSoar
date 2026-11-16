// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "LXListRenderer.hpp"
#include "GradientRenderer.hpp"
#include "Look/LXLook.hpp"
#include "ui/canvas/Canvas.hpp"

#include <algorithm>

namespace LX {

void
DrawListRow(Canvas &canvas, const PixelRect &rc, RowState state) noexcept
{
  const unsigned radius = std::max(1u, Scale(ListRow::RADIUS));

  switch (state) {
  case RowState::NORMAL:
    canvas.DrawFilledRectangle(rc, ListRow::PANEL);
    break;

  case RowState::SELECTED:
    canvas.DrawFilledRectangle(rc, ListRow::PANEL);
    DrawGradientBox(canvas, rc, ListRow::TOP, ListRow::BOTTOM,
                    ListRow::BOTTOM, radius);
    break;

  case RowState::FOCUSED:
    canvas.DrawFilledRectangle(rc, ListRow::PANEL);
    DrawGradientBox(canvas, rc, ListRow::TOP, ListRow::BOTTOM,
                    ListRow::BORDER, radius);
    break;

  case RowState::PRESSED:
    canvas.DrawFilledRectangle(rc, ListRow::PANEL);
    DrawGradientBox(canvas, rc, SELECTION, SELECTION, ListRow::BORDER,
                    radius);
    break;
  }
}

void
DrawScrollBar(Canvas &canvas, const PixelRect &track,
              const PixelRect &thumb) noexcept
{
  const unsigned radius = std::max(1u, Scale(ScrollBar::RADIUS));
  DrawGradientBox(canvas, track, ScrollBar::BACKGROUND,
                  ScrollBar::BACKGROUND, ScrollBar::OUTLINE, radius);

  if (thumb.GetHeight() <= 0)
    return;

  const int inset = std::max(0, (int(track.GetWidth()) -
                                 int(Scale(ScrollBar::THUMB_WIDTH))) / 2);
  PixelRect rc = thumb;
  rc.left = track.left + inset;
  rc.right = track.right - inset;
  DrawGradientBox(canvas, rc, ScrollBar::THUMB, ScrollBar::THUMB,
                  ScrollBar::THUMB, radius);
}

} // namespace LX
