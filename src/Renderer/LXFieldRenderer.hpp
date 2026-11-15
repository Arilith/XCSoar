// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ui/dim/Rect.hpp"

#include <cstdint>

class Canvas;
class Font;
class Color;
struct DialogLook;

namespace LX {

/** How a field of a settings form is drawn. */
enum class FieldState : uint8_t {
  /** An editable value. */
  NORMAL,

  /** An editable value that has the keyboard focus. */
  FOCUSED,

  /** The value is being edited, or the finger is on the field. */
  EDITING,

  /** A value that is shown but cannot be edited. */
  READ_ONLY,

  /** A value that is not available at the moment. */
  DISABLED,
};

/** The colour of text drawn on a field in the given state. */
[[gnu::const]]
Color GetFieldTextColor(FieldState state) noexcept;

/**
 * Draw the face of a field into @p rc.  If the field has the focus,
 * its ring is drawn around @p rc.
 */
void DrawField(Canvas &canvas, const PixelRect &rc,
               FieldState state) noexcept;

/**
 * Draw the value text into a field which has been drawn by
 * DrawField().  The text is clipped to the field.
 *
 * @param warning draw the text in a warning colour (ignored while
 * the field is being edited)
 */
void DrawFieldValue(Canvas &canvas, const PixelRect &rc, FieldState state,
                    const Font &font, const char *text,
                    bool warning) noexcept;

/** Where the label of a field is drawn. */
enum class LabelPlacement : uint8_t {
  /** Right-aligned, left of the field. */
  LEFT_OF_FIELD,

  /** Left-aligned, bottom edge on the field. */
  ABOVE_FIELD,
};

/**
 * Draw the label of a field into @p area, the space the control
 * leaves for it.
 */
void DrawFieldLabel(Canvas &canvas, const PixelRect &area,
                    LabelPlacement placement, FieldState state,
                    const Font &font, const char *label) noexcept;

/**
 * Draw a check box row: a field with the check box glyph and the
 * caption.  The glyph shows the state with a tick, so it does not
 * depend on colour.
 */
void DrawCheckBoxField(Canvas &canvas, const DialogLook &look,
                       const PixelRect &rc, FieldState state,
                       bool checked, const char *caption) noexcept;

} // namespace LX
