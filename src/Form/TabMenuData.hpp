// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include <memory>

class Widget;

/**
 * A tile of the configuration menu.  Exactly one of #Load, #Action,
 * #stack and #children says what the tile does.  Tables of items end with an
 * item without a caption.
 */
struct TabMenuItem {
  const char *caption;

  /** Open a settings page. */
  std::unique_ptr<Widget> (*Load)() = nullptr;

  /** Call a function, e.g. to show a dialog. */
  void (*Action)() = nullptr;

  /**
   * Open one page which stacks the pages of these items, each in a
   * labelled box.  Their settings are saved in this order.
   */
  const TabMenuItem *stack = nullptr;

  /**
   * Open a grid of further tiles.  A grid with a single available
   * tile opens that tile directly.
   */
  const TabMenuItem *children = nullptr;

  /** Called when building the menu; an unavailable item is omitted. */
  bool (*IsAvailable)() = nullptr;

  /**
   * Base name of the icon resource (e.g. "lx_units"); the tile has
   * no icon if the resource was not built.
   */
  const char *icon = nullptr;
};
