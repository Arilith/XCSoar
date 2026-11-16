// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "LXFieldRenderer.hpp"
#include "GradientRenderer.hpp"
#include "Form/CheckBox.hpp"
#include "Look/DialogLook.hpp"
#include "Look/LXLook.hpp"
#include "ui/canvas/Canvas.hpp"
#include "ui/canvas/Font.hpp"

#include <algorithm>

namespace LX {

Color
GetFieldTextColor(FieldState state) noexcept
{
  return state == FieldState::DISABLED
    ? Field::DISABLED_TEXT
    : Field::TEXT;
}

static unsigned
GetRadius() noexcept
{
  return std::max(1u, Scale(Field::RADIUS));
}

static unsigned
GetRingWidth() noexcept
{
  return std::max(1u, Scale(Field::RING_OUTSET));
}

void
DrawField(Canvas &canvas, const PixelRect &rc, FieldState state) noexcept
{
  if (state == FieldState::FOCUSED) {
    PixelRect ring = rc;
    ring.Grow(GetRingWidth());
    DrawGradientBox(canvas, ring, Field::FOCUS_RING, Field::FOCUS_RING,
                    Field::FOCUS_RING, GetRadius());
  }

  switch (state) {
  case FieldState::EDITING:
    DrawGradientBox(canvas, rc, Field::ACTIVE_TOP, Field::ACTIVE_BOTTOM,
                    Field::FOCUS_RING, GetRadius());
    break;

  case FieldState::READ_ONLY:
    DrawGradientBox(canvas, rc, Field::FLAT, Field::FLAT, Field::FLAT,
                    GetRadius());
    break;

  case FieldState::NORMAL:
  case FieldState::FOCUSED:
  case FieldState::DISABLED:
    DrawGradientBox(canvas, rc, Field::TOP, Field::BOTTOM, Field::EDGE,
                    GetRadius());
    break;
  }
}

/** The left edge of the text, and its top for a vertically centred line. */
static PixelPoint
GetTextOrigin(const PixelRect &rc, const Font &font) noexcept
{
  return {rc.left + int(Scale(Field::TEXT_PADDING)),
          rc.top + (int(rc.GetHeight()) - int(font.GetHeight())) / 2};
}

void
DrawFieldValue(Canvas &canvas, const PixelRect &rc, FieldState state,
               const Font &font, const char *text, bool warning) noexcept
{
  if (*text == '\0')
    return;

  canvas.Select(font);
  canvas.SetBackgroundTransparent();

  const PixelPoint origin = GetTextOrigin(rc, font);
  const int right = rc.right - int(Scale(Field::TEXT_PADDING));
  if (origin.x >= right)
    return;

  if (state == FieldState::EDITING) {
    /* the whole value is selected */
    const int width = std::min(int(canvas.CalcTextSize(text).width),
                               right - origin.x);
    const int inset = Scale(Field::HIGHLIGHT_INSET);
    canvas.DrawFilledRectangle({origin.x, rc.top + inset,
                                origin.x + width, rc.bottom - inset},
                               SELECTION);
    canvas.SetTextColor(Field::TEXT);
  } else
    canvas.SetTextColor(warning ? COLOR_INVERSE_RED
                                : GetFieldTextColor(state));

  canvas.DrawClippedText(origin, unsigned(right - origin.x), text);
}

void
DrawFieldLabel(Canvas &canvas, const PixelRect &area,
               LabelPlacement placement, FieldState state,
               const Font &font, const char *label) noexcept
{
  if (*label == '\0' || area.right <= area.left)
    return;

  canvas.Select(font);
  canvas.SetBackgroundTransparent();
  canvas.SetTextColor(GetFieldTextColor(state));

  const PixelSize size = canvas.CalcTextSize(label);
  PixelPoint p;
  switch (placement) {
  case LabelPlacement::LEFT_OF_FIELD:
    p = {std::max(area.left, area.right - int(size.width)),
         area.top + (int(area.GetHeight()) - int(size.height)) / 2};
    break;

  case LabelPlacement::ABOVE_FIELD:
    p = {area.left, area.bottom - int(size.height)};
    break;
  }

  canvas.DrawClippedText(p, unsigned(std::max(0, area.right - p.x)), label);
}

void
DrawCheckBoxField(Canvas &canvas, const DialogLook &look,
                  const PixelRect &rc, FieldState state,
                  bool checked, const char *caption) noexcept
{
  /* the row stands directly on the black background; only the
     focus is marked */
  if (state == FieldState::FOCUSED)
    DrawField(canvas, rc, state);

  const unsigned size = Scale(CheckBox::BOX_SIZE);
  const int left = rc.left + int(Scale(CheckBox::PADDING));
  const int top = rc.top + (int(rc.GetHeight()) - int(size)) / 2;
  const PixelRect box{left, top, left + int(size), top + int(size)};

  DrawCheckBox(canvas, look, box, checked, state == FieldState::FOCUSED,
               false, state != FieldState::DISABLED);

  const Font &font = *look.check_box.font;
  const PixelRect text_rc{box.right + int(Scale(CheckBox::CAPTION_GAP)),
                          rc.top, rc.right, rc.bottom};
  canvas.Select(font);
  canvas.SetBackgroundTransparent();
  canvas.SetTextColor(GetFieldTextColor(state));
  canvas.DrawClippedText({text_rc.left,
                          rc.top + (int(rc.GetHeight()) -
                                    int(font.GetHeight())) / 2},
                         text_rc, caption);
}

} // namespace LX
