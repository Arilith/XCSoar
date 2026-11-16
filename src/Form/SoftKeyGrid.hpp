// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Look/LXLook.hpp"
#include "Renderer/SoftButtonRenderer.hpp"
#include "ui/dim/Rect.hpp"

#include <algorithm>
#include <span>

/**
 * The grid of soft keys on the bottom edge of the LX look: four
 * slots per row, further rows above.  The visible key is smaller than
 * its slot (see #LX::SoftKey); the rectangle returned here is the
 * touch target, a whole slot wide and never shorter than any other
 * control.
 */
class SoftKeyGrid {
  int left;
  int width;
  int bottom;
  unsigned row_height;

public:
  static constexpr unsigned SLOTS = 4;

  /**
   * @param rc the area the rows are put at the bottom of
   */
  explicit SoftKeyGrid(const PixelRect &rc) noexcept
    :width(std::min(int(LX::Scale(LX::GRID_WIDTH)), int(rc.GetWidth()))),
     bottom(rc.bottom),
     row_height(std::max(Layout::GetMinimumControlHeight(),
                         LX::Scale(LX::SoftKey::HEIGHT +
                                   LX::Badge::OVERHANG))) {
    /* the grid is as wide as the short screen edge, but never wider
       than the available space */
    left = rc.left + (int(rc.GetWidth()) - width) / 2;
  }

  /**
   * Decide which slot each key goes to: a confirming key (check
   * badge) in the last slot, a cancelling one (cross badge) in the
   * first, all others in the remaining slots in order.  With more
   * than four keys they simply fill the slots in order.
   *
   * @param slots receives one slot number per key; slot 0..3 is the
   * bottom row, 4..7 the row above it, and so on
   */
  static void AssignSlots(std::span<const SoftButtonRenderer::Badge> badges,
                          unsigned *slots) noexcept;

  /** How many rows @p n keys need. */
  static constexpr unsigned GetRowCount(unsigned n) noexcept {
    return (n + SLOTS - 1) / SLOTS;
  }

  PixelRect GetSlot(unsigned slot) const noexcept {
    return GetSlot(slot / SLOTS, slot % SLOTS);
  }

  unsigned GetRowHeight() const noexcept {
    return row_height;
  }

  /** The area of @p row_count rows. */
  unsigned GetHeight(unsigned row_count) const noexcept {
    return row_count * row_height;
  }

  /** Row 0 is the bottom row. */
  PixelRect GetSlot(unsigned row, unsigned column) const noexcept {
    const auto length = [this](unsigned design){
      return int(design * width / LX::GRID_WIDTH);
    };

    const int centre = left + length(LX::SoftKey::FIRST_SLOT_X +
                                     column * LX::SoftKey::SLOT_PITCH +
                                     LX::SoftKey::WIDTH / 2);
    const int half = length(LX::SoftKey::SLOT_PITCH) / 2;
    const int slot_bottom = bottom - int(row * row_height);
    return {centre - half, slot_bottom - int(row_height),
            centre + half, slot_bottom};
  }
};
