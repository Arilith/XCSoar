// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "LXKeyRenderer.hpp"
#include "GradientRenderer.hpp"
#include "Look/LXLook.hpp"
#include "Widget/LXKeyboardLayout.hpp"
#include "ui/canvas/Canvas.hpp"
#include "ui/canvas/Font.hpp"
#include "ui/canvas/Pen.hpp"

#include <algorithm>

void
LXKeyRenderer::DrawCaption(Canvas &canvas, const PixelRect &face,
                           Color color) const noexcept
{
  canvas.Select(font);
  canvas.SetBackgroundTransparent();
  canvas.SetTextColor(color);

  /* centre the capital letters, not the line: digits and capitals
     sit in the middle of the key, as on the LX */
  const PixelSize size = canvas.CalcTextSize(caption.c_str());
  const int capital = font.GetCapitalHeight();
  const int baseline = (face.top + face.bottom + capital) / 2;
  canvas.DrawText({(face.left + face.right - int(size.width)) / 2,
                   baseline - int(font.GetAscentHeight())},
                  caption.c_str());
}

void
LXKeyRenderer::DrawSpace(Canvas &canvas, const PixelRect &face,
                         Color color) const noexcept
{
  const int width = LX::Scale(LX::Keyboard::SPACE_GLYPH_WIDTH);
  const int height = LX::Scale(LX::Keyboard::SPACE_GLYPH_HEIGHT);
  const int stroke = LX::Scale(LX::Keyboard::SPACE_GLYPH_STROKE);
  const int tick = LX::Scale(LX::Keyboard::SPACE_GLYPH_TICK);

  const int left = (face.left + face.right - width) / 2;
  const int bottom = face.bottom -
    int(LX::Scale(LX::Keyboard::SPACE_GLYPH_BOTTOM_INSET));
  const int top = bottom - height;

  canvas.DrawFilledRectangle({left, bottom - stroke, left + width, bottom},
                             color);
  canvas.DrawFilledRectangle({left, top, left + tick, bottom}, color);
  canvas.DrawFilledRectangle({left + width - tick, top, left + width, bottom},
                             color);
}

void
LXKeyRenderer::DrawBackspace(Canvas &canvas, const PixelRect &face,
                             Color color) const noexcept
{
  const int width = LX::Scale(LX::Keyboard::BACKSPACE_GLYPH_WIDTH);
  const int height = LX::Scale(LX::Keyboard::BACKSPACE_GLYPH_HEIGHT);
  const int left = (face.left + face.right - width) / 2;
  const int top = (face.top + face.bottom - height) / 2;
  const int right = left + width - 1, bottom = top + height - 1;
  const int middle = top + height / 2;
  const int point = left + height / 2;

  canvas.Select(Pen(std::max(1u, LX::Scale(2)), color));

  /* the outline: a tag pointing left */
  canvas.DrawLine({left, middle}, {point, top});
  canvas.DrawLine({point, top}, {right, top});
  canvas.DrawLine({right, top}, {right, bottom});
  canvas.DrawLine({right, bottom}, {point, bottom});
  canvas.DrawLine({point, bottom}, {left, middle});

  /* the cross, centred in the body of the tag */
  const int cx = (point + right) / 2, cy = middle;
  const int arm = height / 4;
  canvas.DrawLine({cx - arm, cy - arm}, {cx + arm, cy + arm});
  canvas.DrawLine({cx - arm, cy + arm}, {cx + arm, cy - arm});
}

void
LXKeyRenderer::DrawButton(Canvas &canvas, const PixelRect &rc,
                          ButtonState state) const noexcept
{
  canvas.DrawFilledRectangle(rc, LX::Keyboard::PANEL);

  const PixelRect face = LX::Keyboard::GetKeyFace(rc);
  const unsigned radius = LX::Scale(LX::Keyboard::RADIUS);

  if (state == ButtonState::DISABLED) {
    /* the face vanishes into the panel; only the label stays */
    const Color color = LX::Keyboard::DISABLED_TEXT;
    if (glyph == Glyph::SPACE)
      DrawSpace(canvas, face, color);
    else if (glyph == Glyph::BACKSPACE)
      DrawBackspace(canvas, face, color);
    else
      DrawCaption(canvas, face, color);
    return;
  }

  /* the line around the face is the focus ring on the focused key;
     a key being pressed is focused, too */
  const bool focused = state == ButtonState::FOCUSED ||
    state == ButtonState::PRESSED;
  PixelRect outline = face;
  outline.Grow(1);
  const Color outline_color = focused ? LX::Keyboard::FOCUS_RING
    : LX::Keyboard::KEY_OUTLINE;
  DrawGradientBox(canvas, outline, outline_color, outline_color,
                  outline_color, radius);

  const bool action = kind == Kind::ACTION;
  const Color fill = action ? LX::Keyboard::ACTION_FILL
    : LX::Keyboard::KEY_FILL;
  DrawGradientBox(canvas, face, fill, fill,
                  action ? LX::Keyboard::ACTION_EDGE : LX::Keyboard::KEY_EDGE,
                  radius);

  if (glyph == Glyph::SPACE)
    DrawSpace(canvas, face, LX::Keyboard::DISABLED_TEXT);
  else if (glyph == Glyph::BACKSPACE)
    DrawBackspace(canvas, face, LX::Keyboard::TEXT);
  else
    DrawCaption(canvas, face, LX::Keyboard::TEXT);
}
