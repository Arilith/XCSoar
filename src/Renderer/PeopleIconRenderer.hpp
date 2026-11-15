// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Screen/DesignGrid.hpp"
#include "ui/canvas/Color.hpp"

class Canvas;

/**
 * Draw a two-person silhouette (the "profile" icon) in a flat
 * colour.  Its top left corner is at the given design coordinates.
 */
void
DrawPeopleIcon(Canvas &canvas, const DesignGrid &grid, unsigned x, unsigned y,
               Color color) noexcept;
