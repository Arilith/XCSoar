// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ui/dim/Rect.hpp"

class Canvas;

namespace LX {

/** How a row of a list is drawn. */
enum class RowState {
  /** An ordinary row: it shows the flat panel. */
  NORMAL,

  /** The cursor row, while the list does not have the focus. */
  SELECTED,

  /** The cursor row of the list that has the focus. */
  FOCUSED,

  /** The finger is on the row. */
  PRESSED,
};

/**
 * Draw the background of one list row into @p rc.  Only Canvas
 * rectangles are used.
 */
void DrawListRow(Canvas &canvas, const PixelRect &rc,
                 RowState state) noexcept;

/**
 * Draw a scroll bar: an outlined track with a white thumb, no
 * arrow buttons.
 *
 * @param track the whole scroll bar
 * @param thumb the part of the track that is the slider
 */
void DrawScrollBar(Canvas &canvas, const PixelRect &track,
                   const PixelRect &thumb) noexcept;

} // namespace LX
