// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ui/window/PaintWindow.hpp"
#include "ui/canvas/Font.hpp"
#include "ui/canvas/Icon.hpp"
#include "util/StaticArray.hxx"

struct TabMenuGroup;
struct DialogLook;
class PagerWidget;

/**
 * The tile grid of the configuration menu (the LXNAV "Setup" menu).
 * Each tile opens the first page of its group.
 */
class TabMenuDisplay final : public PaintWindow
{
  static constexpr unsigned MAX_TILES = 24;
  static constexpr unsigned MAX_PAGES = 64;

  /**
   * The offset from a page number in the #TabMenuDisplay to a page
   * number in the #PagerWidget.
   */
  static constexpr unsigned PAGE_OFFSET = 1;

  struct Tile {
    const char *caption;
    MaskedIcon icon;

    /** Index of the first/last page of this tile in the pager */
    unsigned first_page_index, last_page_index;
  };

  struct PageCaption {
    const char *caption;
    unsigned tile;
  };

  /** Geometry of the grid; see UpdateLayout() */
  struct Metrics {
    unsigned columns = 1;
    PixelSize tile_size{1, 1};
    int pitch_x = 1, pitch_y = 1;
    int left = 0, top = 0;
    unsigned content_height = 0;
  };

  PagerWidget &pager;
  const DialogLook &look;

  StaticArray<Tile, MAX_TILES> tiles;
  StaticArray<PageCaption, MAX_PAGES> pages;

  Metrics metrics;
  Font label_font, title_font;
  const char *title = nullptr;

  /** The focused tile */
  unsigned cursor = 0;

  /** Pixels the grid is scrolled up */
  unsigned scroll = 0;

  /** The tile under the pointer while the mouse button is down */
  unsigned down_tile = NO_TILE;
  static constexpr unsigned NO_TILE = ~0u;

public:
  TabMenuDisplay(PagerWidget &pager, const DialogLook &look) noexcept;

  /**
   * Build the tiles and add their pages to the pager.
   *
   * @param title caption of the menu
   */
  void InitMenu(const TabMenuGroup groups[], unsigned n_groups,
                const char *title) noexcept;

  const char *GetCaption(char buffer[], size_t size) const noexcept;

  /**
   * Call this from PagerWidget's OnPageFlipped callback.  It moves
   * the cursor to the tile of the newly selected page.
   */
  void OnPageFlipped() noexcept;

  void SetCursor(unsigned i) noexcept;

  unsigned GetCursor() const noexcept {
    return cursor;
  }

  /** Pager index of a page; the menu itself is pager page 0. */
  static constexpr unsigned PagerIndex(unsigned page_index) noexcept {
    return PAGE_OFFSET + page_index;
  }

  /** Pager index of the first page of a tile. */
  unsigned GetPagerIndex(unsigned tile) const noexcept {
    return PAGE_OFFSET + tiles[tile].first_page_index;
  }

private:
  void UpdateLayout() noexcept;
  void LoadFonts() noexcept;

  [[gnu::pure]]
  unsigned GetViewHeight() const noexcept;

  /** Rows that fit below the title. */
  [[gnu::pure]]
  unsigned GetVisibleRows() const noexcept;

  [[gnu::pure]]
  PixelRect GetTileRect(unsigned i) const noexcept;

  [[gnu::pure]]
  unsigned FindTile(PixelPoint p) const noexcept;

  void Open(unsigned tile) noexcept;
  void MoveCursor(int delta) noexcept;
  void SetScroll(int value) noexcept;
  void EnsureVisible(unsigned tile) noexcept;

  void DrawTile(Canvas &canvas, unsigned i) const noexcept;
  void DrawTitle(Canvas &canvas) const noexcept;
  void DrawScrollBar(Canvas &canvas) const noexcept;

protected:
  void OnResize(PixelSize new_size) noexcept override;

  bool OnMouseMove(PixelPoint p, unsigned keys) noexcept override;
  bool OnMouseUp(PixelPoint p) noexcept override;
  bool OnMouseDown(PixelPoint p) noexcept override;
  bool OnMouseWheel(PixelPoint p, int delta) noexcept override;
  bool OnKeyCheck(unsigned key_code) const noexcept override;
  bool OnKeyDown(unsigned key_code) noexcept override;
  void OnCancelMode() noexcept override;

  void OnPaint(Canvas &canvas) noexcept override;

  void OnKillFocus() noexcept override;
  void OnSetFocus() noexcept override;
};
