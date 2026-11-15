// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ButtonRenderer.hpp"
#include "Screen/DesignGrid.hpp"
#include "util/StaticString.hxx"

class Font;

/**
 * A "soft key" in the style of the LXNAV LX9000 series: a flat light
 * face with black capital letters and a small coloured badge sitting
 * on its top edge (see #LX::SoftKey).
 *
 * The face is aligned to the bottom and centred in the button
 * rectangle, so the button window can be taller and wider than the
 * face to provide a comfortable touch target.
 */
class SoftButtonRenderer final : public ButtonRenderer {
public:
  enum class Badge {
    NONE,

    /** green check mark: confirm */
    CHECK,

    /** red cross: cancel */
    CROSS,
  };

private:
  const Font *font;
  StaticString<64> caption;
  Badge badge;
  DesignGrid grid;

public:
  SoftButtonRenderer(const Font &_font, const char *_caption, Badge _badge,
                     const DesignGrid &_grid) noexcept
    :font(&_font), caption(_caption), badge(_badge), grid(_grid) {}

  /** Apply a new scale; the font must match it. */
  void SetGrid(const Font &_font, const DesignGrid &_grid) noexcept {
    font = &_font;
    grid = _grid;
  }

  /* virtual methods from class ButtonRenderer */
  unsigned GetMinimumButtonWidth() const noexcept override;
  void DrawButton(Canvas &canvas, const PixelRect &rc,
                  ButtonState state) const noexcept override;

private:
  [[gnu::pure]]
  PixelRect GetFaceRect(const PixelRect &rc) const noexcept;

  void DrawBadge(Canvas &canvas, const PixelRect &face) const noexcept;
  void DrawCaption(Canvas &canvas, const PixelRect &face,
                   ButtonState state) const noexcept;
};
