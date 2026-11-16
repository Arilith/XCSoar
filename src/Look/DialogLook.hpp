// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ButtonLook.hpp"
#include "CheckBoxLook.hpp"
#include "ui/canvas/Color.hpp"
#include "ui/canvas/Pen.hpp"
#include "ui/canvas/Brush.hpp"
#include "ui/canvas/Font.hpp"

#ifdef EYE_CANDY
#include "ui/canvas/Bitmap.hpp"
#endif

struct DialogLook {
  bool dark_mode = false;

  /**
   * Draw dialog controls in the style of the LXNAV LX9000: dark
   * gradient fields, check boxes with a tick, dense forms.  Only on
   * colour displays in dark mode; the light theme and e-paper keep
   * the classic controls.
   */
  bool lx = false;

  struct {
    Color text_color;

    const Font *font;

#ifdef EYE_CANDY
    Bitmap background_bitmap;
#endif

    Color background_color;
    Color inactive_background_color;
  } caption;

  Color background_color, background_gradient_top_color, text_color;

  Brush background_brush;

  Font text_font, bold_font, small_font, small_font_bold;

  /** The font of #caption in the LX look. */
  Font title_font;

  /** The font of the soft keys in the LX look. */
  Font soft_key_font;

  /** The font of the on-screen keyboard keys in the LX look. */
  Font key_font, key_label_font;

  /** Bold font scaled up for H1 headings in rich text */
  Font heading1_font;

  /** Bold font scaled up for H2 headings in rich text */
  Font heading2_font;

  struct {
    Color background_color, text_color;

    Pen border_pen;
  } focused;

  struct {
    Color background_color, text_color;

    struct {
      Color background_color, text_color;
    } selected;

    /**
     * Colors for the selected item when the list has the keyboard
     * focus.
     */
    struct {
      Color background_color, text_color;
    } focused;

    /**
     * Colors for the selected item when the mouse/finger is pressed.
     */
    struct {
      Color background_color, text_color;
    } pressed;

    const Font *font, *font_bold;

    [[gnu::pure]]
    Color GetTextColor(bool is_selected, bool is_focused,
                       bool is_pressed) const {
      return is_pressed
        ? pressed.text_color
        : (is_selected
           ? (is_focused
              ? focused.text_color
              : selected.text_color)
           : text_color);
    }

    [[gnu::pure]]
    Color GetBackgroundColor(bool is_selected, bool is_focused,
                             bool is_pressed) const {
      return is_pressed
        ? pressed.background_color
        : (is_selected
           ? (is_focused
              ? focused.background_color
              : selected.background_color)
           : background_color);
    }
  } list;

  ButtonLook button;
  CheckBoxLook check_box;

  void LoadFonts();

private:
  /** Tahoma-proportioned regular fonts of the LX look. */
  void LoadLXFonts();

public:

  void Initialise(bool dark_mode = false);

  void SetBackgroundColor(Color color);

  /** Unfocused read-only value fill (#WndProperty, #LargeTextWindow). */
  [[gnu::pure]]
  Color ReadOnlyValueBackground() const noexcept;

  [[gnu::pure]]
  Color ReadOnlyValueBorderColor() const noexcept;
};
