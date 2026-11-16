// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ButtonRenderer.hpp"
#include "Screen/DesignGrid.hpp"
#include "util/StaticString.hxx"

#include <optional>

class Font;

/**
 * A "soft key" in the style of the LXNAV LX9000 series: a flat light
 * face with black capital letters and a small coloured badge sitting
 * on its top edge (see #LX::SoftKey).
 *
 * The face is aligned to the bottom and centred in the button
 * rectangle, so the button window can be taller and wider than the
 * face to provide a comfortable touch target.  Focus is shown by a
 * thin white ring around the face.
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

  /** If unset, the grid follows the current screen (#LX::Scale). */
  std::optional<DesignGrid> fixed_grid;

public:
  /** A soft key scaled like #LX::Scale. */
  SoftButtonRenderer(const Font &_font, const char *_caption,
                     Badge _badge) noexcept
    :font(&_font), caption(_caption), badge(_badge) {}

  SoftButtonRenderer(const Font &_font, const char *_caption, Badge _badge,
                     const DesignGrid &_grid) noexcept
    :font(&_font), caption(_caption), badge(_badge), fixed_grid(_grid) {}

  /** Apply a new scale; the font must match it. */
  void SetGrid(const Font &_font, const DesignGrid &_grid) noexcept {
    font = &_font;
    fixed_grid = _grid;
  }

  Badge GetBadge() const noexcept {
    return badge;
  }

  /* virtual methods from class ButtonRenderer */
  void SetCaption(const char *_caption) noexcept override {
    caption = _caption;
  }

  unsigned GetMinimumButtonWidth() const noexcept override;
  void DrawButton(Canvas &canvas, const PixelRect &rc,
                  ButtonState state) const noexcept override;

private:
  [[gnu::pure]]
  DesignGrid GetGrid() const noexcept;

  [[gnu::pure]]
  PixelRect GetFaceRect(const PixelRect &rc,
                        const DesignGrid &grid) const noexcept;

  void DrawBadge(Canvas &canvas, const PixelRect &face,
                 const DesignGrid &grid) const noexcept;
  void DrawCaption(Canvas &canvas, const PixelRect &face,
                   ButtonState state,
                   const DesignGrid &grid) const noexcept;
};
