// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "CheckBoxLook.hpp"
#include "Colors.hpp"
#include "Screen/Layout.hpp"
#include "Look/LXLook.hpp"

#include <algorithm>

void
CheckBoxLook::Initialise(const Font &_font, bool dark_mode, bool lx)
{
  font = &_font;

  const unsigned pen_width = Layout::ScaleFinePenWidth(1);

  if (lx) {
    /* an outline and a tick on the field behind them: no fill, and
       the state is shown by the tick, not by colour */
    const unsigned outline = std::max(1u, LX::Scale(LX::CheckBox::OUTLINE));

    focus_background_brush.Destroy();

    standard.box_brush.Destroy();
    standard.box_pen.Create(outline, LX::Field::TEXT);
    standard.check_brush.Create(LX::Field::TEXT);
    standard.text_color = LX::Field::TEXT;

    focused = standard;
    pressed = standard;

    disabled.box_brush.Destroy();
    disabled.box_pen.Create(outline, LX::Field::DISABLED_TEXT);
    disabled.check_brush.Create(LX::Field::DISABLED_TEXT);
    disabled.text_color = LX::Field::DISABLED_TEXT;
  } else if (dark_mode) {
    focus_background_brush.Create(COLOR_XCSOAR);

    standard.box_brush.Create(COLOR_DARK_THEME_LIST);
    standard.box_pen.Create(pen_width, COLOR_LIGHT_GRAY);
    standard.check_brush.Create(COLOR_WHITE);
    standard.text_color = COLOR_WHITE;

    focused.box_brush.Create(COLOR_DARK_THEME_LIST);
    focused.box_pen.Create(pen_width, COLOR_WHITE);
    focused.check_brush.Create(COLOR_WHITE);
    focused.text_color = COLOR_WHITE;

    pressed.box_brush.Create(DarkColor(COLOR_XCSOAR_LIGHT));
    pressed.box_pen.Create(pen_width, COLOR_WHITE);
    pressed.check_brush.Create(COLOR_WHITE);
    pressed.text_color = COLOR_WHITE;

    disabled.box_brush.Create(COLOR_DARK_THEME_LIST);
    disabled.box_pen.Create(pen_width, COLOR_GRAY);
    disabled.check_brush.Create(COLOR_GRAY);
    disabled.text_color = COLOR_GRAY;
  } else {
    focus_background_brush.Create(COLOR_XCSOAR_DARK);

    standard.box_brush.Create(COLOR_WHITE);
    standard.box_pen.Create(pen_width, COLOR_BLACK);
    standard.check_brush.Create(COLOR_BLACK);
    standard.text_color = COLOR_BLACK;

    focused.box_brush.Create(COLOR_WHITE);
    focused.box_pen.Create(pen_width, COLOR_BLACK);
    focused.check_brush.Create(COLOR_BLACK);
    focused.text_color = COLOR_WHITE;

    pressed.box_brush.Create(COLOR_XCSOAR_LIGHT);
    pressed.box_pen.Create(pen_width, COLOR_BLACK);
    pressed.check_brush.Create(COLOR_BLACK);
    pressed.text_color = COLOR_WHITE;

    disabled.box_brush.Create(COLOR_WHITE);
    disabled.box_pen.Create(pen_width, COLOR_GRAY);
    disabled.check_brush.Create(COLOR_GRAY);
    disabled.text_color = COLOR_GRAY;
  }
}
