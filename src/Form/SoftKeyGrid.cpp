// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "SoftKeyGrid.hpp"

void
SoftKeyGrid::AssignSlots(std::span<const SoftButtonRenderer::Badge> badges,
                         unsigned *slots) noexcept
{
  const unsigned n = badges.size();
  if (n > SLOTS) {
    for (unsigned i = 0; i < n; ++i)
      slots[i] = i;
    return;
  }

  constexpr unsigned NONE = ~0u;
  bool taken[SLOTS] = {};
  const auto place = [&](unsigned i, unsigned slot){
    slots[i] = slot;
    taken[slot] = true;
  };

  for (unsigned i = 0; i < n; ++i)
    slots[i] = NONE;

  const auto claim = [&](SoftButtonRenderer::Badge badge, unsigned slot){
    for (unsigned i = 0; i < n; ++i)
      if (badges[i] == badge) {
        place(i, slot);
        return;
      }
  };
  claim(SoftButtonRenderer::Badge::CHECK, SLOTS - 1);
  claim(SoftButtonRenderer::Badge::CROSS, 0);

  unsigned next = 0;
  for (unsigned i = 0; i < n; ++i) {
    if (slots[i] != NONE)
      continue;

    while (taken[next])
      ++next;
    place(i, next);
  }
}
