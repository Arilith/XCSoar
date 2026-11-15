// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ui/dim/Rect.hpp"

#include <algorithm>
#include <cmath>

/**
 * Maps a fixed design grid (e.g. 480x788 pixels) onto a window: the
 * grid is scaled uniformly to fit and centred horizontally, so
 * proportions survive any window size.
 */
class DesignGrid {
  double scale = 1;
  int left = 0;

public:
  DesignGrid() noexcept = default;

  DesignGrid(PixelSize window, PixelSize design) noexcept
    :scale(std::min(double(window.width) / design.width,
                    double(window.height) / design.height)) {
    left = (int(window.width) - Scale(design.width)) / 2;
  }

  /** A design length in window pixels. */
  [[gnu::pure]]
  int Scale(double design) const noexcept {
    return int(std::lround(design * scale));
  }

  /** A design x coordinate in window pixels. */
  [[gnu::pure]]
  int X(double design) const noexcept {
    return left + Scale(design);
  }

  /** A design y coordinate in window pixels. */
  [[gnu::pure]]
  int Y(double design) const noexcept {
    return Scale(design);
  }

  /** A rectangle given by design coordinates (right/bottom exclusive). */
  [[gnu::pure]]
  PixelRect Rect(double l, double t, double r, double b) const noexcept {
    return {X(l), Y(t), X(r), Y(b)};
  }
};
