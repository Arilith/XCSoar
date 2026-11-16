// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Look/LXLook.hpp"
#include "ui/dim/Rect.hpp"

#include <algorithm>

/**
 * Where the on-screen keyboard of the LX look goes, in the units of
 * the design grid (see #LX::Keyboard).
 */
namespace LX::Keyboard {

/**
 * The keyboard panel: centred in @p rc, just above its bottom edge.
 *
 * @param rc the area above the soft keys
 */
[[gnu::const]]
static inline PixelRect
GetPanelRect(const PixelRect &rc) noexcept
{
  const int width = std::min(int(Scale(PANEL_WIDTH)), int(rc.GetWidth()));
  const int height = std::min(int(Scale(PANEL_HEIGHT)), int(rc.GetHeight()));
  const int left = rc.left + (int(rc.GetWidth()) - width) / 2;
  const int bottom = rc.bottom - int(Scale(PANEL_BOTTOM_GAP));
  return {left, bottom - height, left + width, bottom};
}

/**
 * The cell of a key: the area which reacts to a touch.  The visible
 * face is centred in it.  The cells of a keyboard are centred in the
 * panel.
 *
 * @param span the number of columns the key covers
 */
[[gnu::const]]
static inline PixelRect
GetKeyCell(const PixelRect &panel, unsigned row, unsigned column,
           unsigned span=1) noexcept
{
  const int pitch_x = Scale(COLUMN_PITCH);
  const int pitch_y = Scale(ROW_PITCH);
  const int left = panel.left +
    (int(panel.GetWidth()) - int(COLUMNS) * pitch_x) / 2;
  const int top = panel.top +
    (int(panel.GetHeight()) - int(ROWS) * pitch_y) / 2;

  return {left + int(column) * pitch_x, top + int(row) * pitch_y,
          left + int(column + span) * pitch_x, top + int(row + 1) * pitch_y};
}

/** The visible face of a key within its cell. */
[[gnu::const]]
static inline PixelRect
GetKeyFace(const PixelRect &cell) noexcept
{
  const int inset_x = (int(Scale(COLUMN_PITCH)) - int(Scale(KEY_WIDTH))) / 2;
  const int inset_y = (int(Scale(ROW_PITCH)) - int(Scale(KEY_HEIGHT))) / 2;
  return {cell.left + inset_x, cell.top + inset_y,
          cell.right - inset_x, cell.bottom - inset_y};
}

} // namespace LX::Keyboard
