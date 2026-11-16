// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Widget.hpp"
#include "ui/dim/Rect.hpp"
#include "ui/window/PaintWindow.hpp"

#include <memory>

struct DialogLook;

/**
 * Draws the group box of a #SectionWidget: a thin outline with the
 * caption sitting on its top edge.
 */
class SectionFrame final : public PaintWindow {
  const DialogLook &look;
  const char *const caption;

public:
  SectionFrame(const DialogLook &_look, const char *_caption) noexcept
    :look(_look), caption(_caption) {}

protected:
  void OnPaint(Canvas &canvas) noexcept override;
};

/**
 * A #Widget decorator which shows another widget in a labelled group
 * box, like the "Elements" box of the LXNAV LX9000 forms.  All
 * methods are forwarded to the hosted widget.
 */
class SectionWidget final : public Widget {
  const DialogLook &look;
  const std::unique_ptr<Widget> widget;
  SectionFrame frame;

public:
  SectionWidget(const DialogLook &_look, const char *caption,
                std::unique_ptr<Widget> &&_widget) noexcept
    :look(_look), widget(std::move(_widget)), frame(_look, caption) {}

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
  /** The rectangle of the hosted widget inside the group box. */
  [[gnu::pure]]
  PixelRect GetContentRect(const PixelRect &rc) const noexcept;

  /** Height and width that the box adds around the hosted widget. */
  [[gnu::pure]]
  PixelSize GetFrameSize() const noexcept;
};
