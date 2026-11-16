// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ui/window/PaintWindow.hpp"
#include "ui/canvas/Font.hpp"
#include "ui/canvas/Icon.hpp"
#include "util/StaticArray.hxx"

#include <functional>
#include <memory>

struct TabMenuItem;
struct DialogLook;
class PagerWidget;
class Widget;

/**
 * The tile grid of the configuration menu (the LXNAV "Setup" menu).
 * A tile opens a settings page, calls a function or opens a grid of
 * further tiles (see #TabMenuItem).
 */
class TabMenuDisplay final : public PaintWindow
{
  static constexpr unsigned MAX_NODES = 128;
  static constexpr unsigned MAX_PAGES = 64;
  static constexpr unsigned NONE = ~0u;

  /**
   * The offset from a page number in the #TabMenuDisplay to a page
   * number in the #PagerWidget.
   */
  static constexpr unsigned PAGE_OFFSET = 1;

  /**
   * A tile.  The tiles of one grid are consecutive; the root node is
   * the menu itself and its children are the tiles of the main grid.
   */
  struct Node {
    const char *caption;
    MaskedIcon icon;

    std::unique_ptr<Widget> (*load)();
    void (*action)();

    /** Index of the page in the pager, if this tile has a page */
    unsigned page;

    unsigned parent;
    unsigned first_child, num_children;

    /** The focused child, if this tile is a grid */
    unsigned focus;
  };

  /** Geometry of the grid; see UpdateLayout() */
  struct Metrics {
    unsigned columns = 1;
    PixelSize tile_size{1, 1};
    int pitch_x = 1, pitch_y = 1;
    int left = 0, top = 0;
  };

  PagerWidget &pager;
  const DialogLook &look;

  StaticArray<Node, MAX_NODES> nodes;

  /** The node of each page in the pager */
  StaticArray<unsigned, MAX_PAGES> page_nodes;

  /** The node whose children are shown */
  unsigned current = 0;

  Metrics metrics;
  Font label_font;

  /** Pixels the grid is scrolled up; a multiple of the row pitch */
  unsigned scroll = 0;

  /** The tile under the pointer while the mouse button is down */
  unsigned down_tile = NONE;

  std::function<void()> level_callback;

public:
  TabMenuDisplay(PagerWidget &pager, const DialogLook &look) noexcept;

  /**
   * Build the menu and add its pages to the pager.
   *
   * @param items the tiles of the main grid
   * @param title caption of the main grid
   */
  void InitMenu(const TabMenuItem *items, const char *title) noexcept;

  /** The caption of the menu, or of the current page. */
  const char *GetCaption() const noexcept;

  /**
   * Call this from PagerWidget's OnPageFlipped callback.  It moves
   * the cursor to the tile of the newly selected page.
   */
  void OnPageFlipped() noexcept;

  /** Called when another grid is shown. */
  void SetLevelCallback(std::function<void()> callback) noexcept {
    level_callback = std::move(callback);
  }

  /** Is a grid shown below the main grid? */
  bool IsSubGrid() const noexcept {
    return current != 0;
  }

  /** Go back to the grid above.  @return false in the main grid */
  bool LeaveSubGrid() noexcept;

  /** The focused tile of the main grid. */
  void SetCursor(unsigned i) noexcept;
  unsigned GetCursor() const noexcept {
    return nodes[0].focus;
  }

  /** Pager index of the page with the given loader; asserts it exists. */
  [[gnu::pure]]
  unsigned FindPage(std::unique_ptr<Widget> (*load)()) const noexcept;

private:
  void Build(const TabMenuItem *items, unsigned parent) noexcept;

  [[gnu::pure]]
  unsigned GetNumTiles() const noexcept {
    return nodes[current].num_children;
  }

  unsigned &Cursor() noexcept {
    return nodes[current].focus;
  }

  [[gnu::pure]]
  unsigned GetCursorTile() const noexcept {
    return nodes[current].focus;
  }

  [[gnu::pure]]
  const Node &GetTile(unsigned i) const noexcept {
    return nodes[nodes[current].first_child + i];
  }

  void UpdateLayout() noexcept;
  void LoadFonts() noexcept;

  [[gnu::pure]]
  unsigned GetViewHeight() const noexcept;

  /** Rows that fit below the title. */
  [[gnu::pure]]
  unsigned GetVisibleRows() const noexcept;

  [[gnu::pure]]
  unsigned GetNumRows() const noexcept;

  [[gnu::pure]]
  PixelRect GetTileRect(unsigned i) const noexcept;

  [[gnu::pure]]
  unsigned FindTile(PixelPoint p) const noexcept;

  void Open(unsigned node) noexcept;
  void EnterGrid(unsigned node) noexcept;
  void SetCurrent(unsigned node) noexcept;
  void MoveCursor(int delta) noexcept;
  void SetScroll(int value) noexcept;
  void EnsureVisible(unsigned tile) noexcept;

  void DrawTile(Canvas &canvas, unsigned i) const noexcept;
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
