// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Widget.hpp"
#include "ui/dim/Rect.hpp"

#include <memory>
#include <vector>

/**
 * A #Widget that stacks several widgets vertically, each as high as
 * it asks to be.  The widgets are laid out at their minimum height,
 * so the caller (usually a #VScrollWidget) scrolls the result if it
 * does not fit.  Widgets are saved in the order they were added.
 */
class StackWidget final : public Widget {
  std::vector<std::unique_ptr<Widget>> widgets;

  PixelRect rc;

public:
  void Add(std::unique_ptr<Widget> widget) noexcept {
    widgets.push_back(std::move(widget));
  }

  /* virtual methods from class Widget */
  PixelSize GetMinimumSize() const noexcept override;
  PixelSize GetMaximumSize() const noexcept override;
  void Initialise(ContainerWindow &parent, const PixelRect &rc) noexcept override;
  void Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept override;
  void Unprepare() noexcept override;
  bool Save(bool &changed) noexcept override;
  bool Click() noexcept override;
  void ReClick() noexcept override;
  void Show(const PixelRect &rc) noexcept override;
  bool Leave() noexcept override;
  void Hide() noexcept override;
  void Move(const PixelRect &rc) noexcept override;
  bool SetFocus() noexcept override;
  bool HasFocus() const noexcept override;
  bool KeyPress(unsigned key_code) noexcept override;

private:
  /**
   * Move all widgets below each other.  Their height depends on
   * their width, so the layout is applied twice.
   */
  void Layout() noexcept;
};
