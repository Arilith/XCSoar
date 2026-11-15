// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Form/Button.hpp"
#include "Screen/DesignGrid.hpp"
#include "ui/canvas/Font.hpp"
#include "ui/window/ContainerWindow.hpp"

#include <functional>
#include <string>
#include <vector>

class FileDataField;

/**
 * The startup screen for choosing a profile, after the LXNAV LX9000
 * "Profile" screen: the profile field is always selected, below it a
 * summary of the selected profile, and Quit and Select soft keys at
 * the bottom.
 */
class ProfileScreen final : public ContainerWindow {
  FileDataField &profiles;

  const std::function<void()> on_field, on_quit, on_select;

  std::vector<std::string> summary;

  DesignGrid grid;
  Font title_font, value_font, info_font, soft_key_font;
  Button quit_button, select_button;

  PixelRect field_rc;
  bool field_pressed = false;

public:
  /**
   * @param on_field called when the field is tapped
   */
  ProfileScreen(FileDataField &_profiles, std::function<void()> _on_field,
                std::function<void()> _on_quit,
                std::function<void()> _on_select) noexcept
    :profiles(_profiles), on_field(std::move(_on_field)),
     on_quit(std::move(_on_quit)), on_select(std::move(_on_select)) {}

  /**
   * Select the next (delta=1) or previous (delta=-1) profile; the
   * list does not wrap around.
   */
  void Step(int delta) noexcept;

  /** The selection was changed from outside. */
  void OnSelectionChanged() noexcept;

protected:
  /* virtual methods from class Window */
  void OnCreate() override;
  void OnResize(PixelSize new_size) noexcept override;
  bool OnMouseDown(PixelPoint p) noexcept override;
  bool OnMouseUp(PixelPoint p) noexcept override;
  void OnCancelMode() noexcept override;
  void OnPaint(Canvas &canvas) noexcept override;

private:
  void LoadFonts() noexcept;
  void LayoutSoftKeys(PixelSize size) noexcept;
  void DrawText(Canvas &canvas, const Font &font, int x, unsigned baseline,
                const char *text, int max_width = 16384) const noexcept;
  void DrawField(Canvas &canvas) const noexcept;
  void DrawSummary(Canvas &canvas) const noexcept;
};
