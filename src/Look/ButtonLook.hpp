// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ui/canvas/Color.hpp"
#include "ui/canvas/Brush.hpp"
#include "ui/canvas/Pen.hpp"
#include "Screen/Layout.hpp"

class Font;

struct ButtonLook {
  const Font *font;

  struct StateLook {
    Color foreground_color;
    Brush foreground_brush;

    Color background_color;
    Pen light_border_pen, dark_border_pen;
    Brush light_border_brush, dark_border_brush;

    void CreateBorder(Color light, Color dark) {
      light_border_pen.Create(Layout::ScaleFinePenWidth(1), light);
      light_border_brush.Create(light);
      dark_border_pen.Create(Layout::ScaleFinePenWidth(1), dark);
      dark_border_brush.Create(dark);
    }
  } standard, selected, focused, pressed;

  /** Dark dialogs use a flat frame instead of a raised bevel. */
  bool flat;

  /**
   * Show captioned buttons as LX soft keys on the bottom edge (see
   * #ButtonPanel); needs #soft_key_font.
   */
  bool lx = false;
  const Font *soft_key_font = nullptr;

  struct {
    Color color;
    Brush brush;
  } disabled;

  void Initialise(const Font &_font, bool dark_mode = false);
};
