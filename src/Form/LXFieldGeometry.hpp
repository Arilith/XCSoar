// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Look/LXLook.hpp"
#include "ui/dim/Rect.hpp"

#include <algorithm>

namespace LX {

/**
 * Where a field sits inside the rectangle of its control.  All
 * values are pixels; FromLook() converts the design grid.
 */
struct FieldGeometry {
  /** Height of the field face. */
  unsigned field_height;

  /** Width of the focus ring around the field. */
  unsigned ring;

  /** Height of a row with the label left of the field. */
  unsigned row_pitch;

  static FieldGeometry FromLook() noexcept {
    return {Scale(Field::HEIGHT), std::max(1u, Scale(Field::RING_OUTSET)),
            Scale(Form::ROW_PITCH)};
  }

  /**
   * The height of a control: a full row, or a field with the label
   * above it.  Never less than @p minimum, the touch target.
   */
  [[gnu::pure]]
  unsigned GetHeight(bool label_above, unsigned label_height,
                     unsigned minimum) const noexcept {
    const unsigned height = label_above
      ? label_height + row_pitch
      : row_pitch;
    return std::max(height, minimum);
  }

  /**
   * The field inside the control rectangle @p rc.  With a negative
   * @p label_width, the label is above the field, which then sits
   * on the bottom edge of the control; otherwise the field starts
   * after the label and is centred vertically.
   */
  [[gnu::pure]]
  PixelRect GetFieldRect(const PixelRect &rc, int label_width,
                         unsigned label_height) const noexcept {
    PixelRect field = rc;
    field.Grow(-int(ring), 0);

    const int available = std::max(1, int(rc.GetHeight()) - 2 * int(ring));
    const int height = std::min(available, int(field_height));

    if (label_width < 0) {
      field.bottom = rc.bottom - int(ring);
      field.top = std::max(field.bottom - height,
                           rc.top + int(label_height));
    } else {
      field.left = rc.left + std::min(std::max(label_width, int(ring)),
                                      int(rc.GetWidth()));
      field.top = rc.top + (int(rc.GetHeight()) - height) / 2;
      field.bottom = field.top + height;
    }

    return field;
  }
};

} // namespace LX
