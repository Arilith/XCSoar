// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include <memory>

class Widget;

struct TabMenuPage {
  const char *menu_caption;

  std::unique_ptr<Widget> (*Load)();

  /** Called when building the menu; an unavailable page is omitted. */
  bool (*IsAvailable)() = nullptr;
};

/** A tile of the menu; opens the first of its pages. */
struct TabMenuGroup {
  const char *caption;

  const TabMenuPage *pages;

  /**
   * Base name of the icon resource (e.g. "lx_units"); the tile has
   * no icon if the resource was not built.
   */
  const char *icon = nullptr;

  /**
   * A tile without pages of its own opens the first page of this
   * other tile (an index into the group table).
   */
  int alias = -1;
};
