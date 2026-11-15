// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "RowFlow.hpp"

#include <algorithm>
#include <cassert>

namespace RowFlow {

/** The width of one column; the columns share the space equally. */
static constexpr int
GetCellWidth(unsigned columns, unsigned width, unsigned margin,
             unsigned gap) noexcept
{
  const int inner = int(width) - 2 * int(margin) - int(gap * (columns - 1));
  return std::max(0, inner) / int(columns);
}

unsigned
CountColumns(unsigned width, unsigned margin, unsigned gap,
             unsigned min_cell_width) noexcept
{
  for (unsigned columns = MAX_COLUMNS; columns > 1; --columns)
    if (GetCellWidth(columns, width, margin, gap) >= int(min_cell_width))
      return columns;

  return 1;
}

unsigned
Assign(std::span<const bool> shareable, unsigned columns,
       std::span<Slot> slots) noexcept
{
  assert(columns >= 1);
  assert(shareable.size() == slots.size());

  unsigned line = 0;

  for (std::size_t i = 0; i < shareable.size();) {
    /* a run of shareable rows */
    std::size_t end = i;
    while (end < shareable.size() && shareable[end])
      ++end;

    if (end == i) {
      slots[i++] = {line++, 0, columns};
      continue;
    }

    /* complete lines first ... */
    for (; end - i >= columns; ++line)
      for (unsigned column = 0; column < columns; ++column)
        slots[i++] = {line, column, 1};

    /* ... and what is left gets a line of its own */
    for (; i < end; ++i)
      slots[i] = {line++, 0, columns};
  }

  return line;
}

Extent
GetExtent(const Slot &slot, unsigned columns, unsigned width,
          unsigned margin, unsigned gap) noexcept
{
  const int cell = GetCellWidth(columns, width, margin, gap);
  const int left = int(margin) + int(slot.column) * (cell + int(gap));

  /* the last column takes the rounding error */
  const int right = slot.column + slot.span >= columns
    ? int(width) - int(margin)
    : left + int(slot.span) * cell + int(slot.span - 1) * int(gap);

  return {left, right};
}

} // namespace RowFlow
