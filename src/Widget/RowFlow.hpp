// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include <span>

/**
 * Arranges the rows of a form in columns.  Short rows that follow
 * each other share a line, side by side; all other rows span the
 * whole width.  The rows keep their order, so reading the form line
 * by line, left to right, visits the rows in the order they were
 * added.  This is the order of the keyboard focus.
 *
 * These are pure functions; #RowFormWidget applies them.
 */
namespace RowFlow {

/** The most columns a form can have. */
static constexpr unsigned MAX_COLUMNS = 2;

/** The place of a row in the form. */
struct Slot {
  /** Index of the line, counted from the top. */
  unsigned line;

  /** The first column of the row. */
  unsigned column;

  /** The number of columns the row spans. */
  unsigned span;

  constexpr bool operator==(const Slot &) const noexcept = default;
};

/** The horizontal extent of a row. */
struct Extent {
  int left, right;

  constexpr bool operator==(const Extent &) const noexcept = default;
};

/**
 * How many columns fit into @p width, if every column must be at
 * least @p min_cell_width wide.
 *
 * @param margin the space left and right of the form
 * @param gap the space between two columns
 */
[[gnu::const]]
unsigned
CountColumns(unsigned width, unsigned margin, unsigned gap,
             unsigned min_cell_width) noexcept;

/**
 * Assign the rows to lines.  Each run of consecutive rows which
 * are marked as shareable is filled line by line; a row that is
 * left over, and every row which is not shareable, spans all
 * columns.
 *
 * @param shareable one flag per row: may this row share its line?
 * @param slots the result, one entry per row
 * @return the number of lines
 */
unsigned
Assign(std::span<const bool> shareable, unsigned columns,
       std::span<Slot> slots) noexcept;

/**
 * The horizontal extent of a row in a form of @p width pixels with
 * @p columns columns.
 */
[[gnu::const]]
Extent
GetExtent(const Slot &slot, unsigned columns, unsigned width,
          unsigned margin, unsigned gap) noexcept;

} // namespace RowFlow
