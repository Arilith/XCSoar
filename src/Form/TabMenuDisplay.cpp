// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "TabMenuDisplay.hpp"
#include "TabMenuData.hpp"
#include "Widget/PagerWidget.hpp"
#include "Widget/VScrollWidget.hpp"
#include "Screen/Layout.hpp"
#include "ui/event/KeyCode.hpp"
#include "ui/canvas/Canvas.hpp"
#include "Look/DialogLook.hpp"
#include "Look/FontDescription.hpp"
#include "Look/LXLook.hpp"
#include "Renderer/GradientRenderer.hpp"
#include "ResourceLookup.hpp"
#include "Language/Language.hpp"
#include "util/StringFormat.hpp"
#include "Asset.hpp"

#include <algorithm>
#include <cassert>
#include <string>

TabMenuDisplay::TabMenuDisplay(PagerWidget &_pager,
                               const DialogLook &_look) noexcept
  :pager(_pager), look(_look)
{
}

/**
 * Load the icon resource of a tile.  The icons are optional: without
 * the resource the icon stays undefined.
 */
static void
LoadTileIcon(MaskedIcon &icon, const char *name) noexcept
{
  if (name == nullptr)
    return;

  std::string id_name{"IDB_"};
  for (const char *p = name; *p != 0; ++p)
    id_name.push_back(char(std::toupper((unsigned char)*p)));

  const auto lookup = [&id_name](const char *suffix){
    return LookupResourceByName((id_name + suffix).c_str());
  };

  const ResourceId id = lookup("");
  if (!id.IsDefined())
    return;

  icon.LoadResource(id, lookup("_MDPI"), lookup("_XHDPI"),
                    lookup("_XXHDPI"));
  icon.KeepColours();
}

void
TabMenuDisplay::InitMenu(const TabMenuGroup groups[], unsigned n_groups,
                         const char *_title) noexcept
{
  assert(groups != nullptr);
  assert(n_groups <= MAX_TILES);

  title = _title;

  /* a tile without a page of its own is built, but opens the page of
     its alias */
  for (unsigned i = 0; i < n_groups; i++) {
    const auto &g = groups[i];
    auto &tile = tiles.append();
    tile.caption = gettext(g.caption);
    LoadTileIcon(tile.icon, g.icon);

    tile.first_page_index = pages.size();
    for (auto p = g.pages; p->Load != nullptr; ++p) {
      if (p->IsAvailable != nullptr && !p->IsAvailable())
        continue;

      auto &page = pages.append();
      page.caption = gettext(p->menu_caption);
      page.tile = i;

      /* Wrap panel in VScrollWidget for automatic scrolling when needed */
      pager.Add(std::make_unique<VScrollWidget>(p->Load(), look));
    }

    tile.last_page_index = pages.size() - 1;
  }

  for (unsigned i = 0; i < n_groups; i++) {
    auto &tile = tiles[i];
    if (tile.last_page_index + 1 == tile.first_page_index &&
        groups[i].alias >= 0) {
      tile.first_page_index = tiles[groups[i].alias].first_page_index;
      tile.last_page_index = tiles[groups[i].alias].last_page_index;
    }
  }
}

const char *
TabMenuDisplay::GetCaption(char buffer[], size_t size) const noexcept
{
  const unsigned page = pager.GetCurrentIndex();
  if (page < PAGE_OFFSET)
    return nullptr;

  const auto &p = pages[page - PAGE_OFFSET];
  StringFormat(buffer, size, "%s > %s", tiles[p.tile].caption, p.caption);
  return buffer;
}

void
TabMenuDisplay::LoadFonts() noexcept
{
  label_font.Load(FontDescription(LX::Scale(LX::Tile::LABEL_FONT)));
  title_font.Load(FontDescription(LX::Scale(LX::TITLE_FONT)));
}

unsigned
TabMenuDisplay::GetViewHeight() const noexcept
{
  return GetSize().height;
}

void
TabMenuDisplay::UpdateLayout() noexcept
{
  const auto window_size = GetSize();
  Metrics m;

  unsigned gap, margin, gutter, min_width;
  if (look.lx) {
    using namespace LX::Tile;
    gap = std::max(1u, LX::Scale(GAP));
    margin = std::max(1u, LX::Scale(MARGIN));
    gutter = LX::Scale(GUTTER);
    min_width = LX::Scale(WIDTH);
    m.tile_size.height = LX::Scale(HEIGHT);
    m.top = LX::Scale(GRID_TOP);
  } else {
    gap = margin = Layout::Scale(2);
    gutter = 0;
    min_width = Layout::GetMinimumControlHeight() * 3;
    m.tile_size.height = Layout::GetMaximumControlHeight();
    m.top = gap;
  }

  /* as many columns as fit, with the tiles stretched to fill them */
  const unsigned avail = window_size.width > gutter + margin
    ? window_size.width - gutter - margin : 1;
  m.columns = std::max(1u, (avail + gap) / (min_width + gap));
  m.tile_size.width = std::max(1u, (avail - gap * (m.columns - 1)) /
                               m.columns);
  m.left = margin;
  m.pitch_x = m.tile_size.width + gap;
  m.pitch_y = m.tile_size.height + gap;

  const unsigned rows = (tiles.size() + m.columns - 1) / m.columns;
  m.content_height = m.top + rows * m.pitch_y;

  metrics = m;
  LoadFonts();
  SetScroll(scroll);
}

PixelRect
TabMenuDisplay::GetTileRect(unsigned i) const noexcept
{
  const int x = metrics.left + int(i % metrics.columns) * metrics.pitch_x;
  const int y = metrics.top + int(i / metrics.columns) * metrics.pitch_y -
    int(scroll);
  return {x, y, x + int(metrics.tile_size.width),
          y + int(metrics.tile_size.height)};
}

unsigned
TabMenuDisplay::FindTile(PixelPoint p) const noexcept
{
  if (p.y < metrics.top)
    return NO_TILE;

  for (unsigned i = 0; i < tiles.size(); ++i)
    if (GetTileRect(i).Contains(p) && GetTileRect(i).bottom <= int(GetViewHeight()))
      return i;

  return NO_TILE;
}

unsigned
TabMenuDisplay::GetVisibleRows() const noexcept
{
  /* partial rows are not drawn: a window does not clip its drawing */
  const int available = int(GetViewHeight()) - metrics.top;
  return std::max(1, (available + metrics.pitch_y -
                      int(metrics.tile_size.height)) / metrics.pitch_y);
}

void
TabMenuDisplay::SetScroll(int value) noexcept
{
  /* the grid scrolls by whole rows */
  const unsigned rows = (tiles.size() + metrics.columns - 1) / metrics.columns;
  const unsigned visible = GetVisibleRows();
  const int max_row = rows > visible ? rows - visible : 0;
  const int row = std::clamp((value + metrics.pitch_y / 2) / metrics.pitch_y,
                             0, max_row);
  if (unsigned(row * metrics.pitch_y) != scroll) {
    scroll = row * metrics.pitch_y;
    Invalidate();
  }
}

void
TabMenuDisplay::EnsureVisible(unsigned tile) noexcept
{
  const int row = tile / metrics.columns;
  const int first = scroll / metrics.pitch_y;
  const int visible = GetVisibleRows();
  if (row < first)
    SetScroll(row * metrics.pitch_y);
  else if (row >= first + visible)
    SetScroll((row - visible + 1) * metrics.pitch_y);
}

void
TabMenuDisplay::OnPageFlipped() noexcept
{
  const unsigned i = pager.GetCurrentIndex();
  if (i >= PAGE_OFFSET && i - PAGE_OFFSET < pages.size())
    SetCursor(pages[i - PAGE_OFFSET].tile);
}

void
TabMenuDisplay::SetCursor(unsigned i) noexcept
{
  if (tiles.empty())
    return;

  i = std::min<unsigned>(i, tiles.size() - 1);
  if (i == cursor)
    return;

  cursor = i;
  if (IsDefined()) {
    EnsureVisible(i);
    Invalidate();
  }
}

void
TabMenuDisplay::OnResize(PixelSize new_size) noexcept
{
  PaintWindow::OnResize(new_size);
  UpdateLayout();
}

void
TabMenuDisplay::Open(unsigned tile) noexcept
{
  cursor = tile;
  pager.ClickPage(GetPagerIndex(tile));
}

void
TabMenuDisplay::MoveCursor(int delta) noexcept
{
  const int next = int(cursor) + delta;
  if (next >= 0 && next < int(tiles.size()))
    SetCursor(next);
  else if (delta > 0 && cursor / metrics.columns <
           (tiles.size() - 1) / metrics.columns)
    /* moving down from a column that is missing in the last row */
    SetCursor(tiles.size() - 1);
}

bool
TabMenuDisplay::OnKeyCheck(unsigned key_code) const noexcept
{
  switch (key_code) {
  case KEY_RETURN:
  case KEY_LEFT:
  case KEY_RIGHT:
    return true;

  case KEY_UP:
    return cursor >= metrics.columns;

  case KEY_DOWN:
    return cursor / metrics.columns < (tiles.size() - 1) / metrics.columns;

  default:
    return false;
  }
}

bool
TabMenuDisplay::OnKeyDown(unsigned key_code) noexcept
{
  const int columns = metrics.columns;
  switch (key_code) {
  case KEY_RETURN:
    Open(cursor);
    return true;

  case KEY_LEFT:
    if (cursor % columns > 0)
      MoveCursor(-1);
    return true;

  case KEY_RIGHT:
    if (cursor % columns + 1 < unsigned(columns))
      MoveCursor(1);
    return true;

  case KEY_UP:
    MoveCursor(-columns);
    return true;

  case KEY_DOWN:
    MoveCursor(columns);
    return true;

  default:
    return false;
  }
}

bool
TabMenuDisplay::OnMouseDown(PixelPoint p) noexcept
{
  SetFocus();

  down_tile = FindTile(p);
  if (down_tile == NO_TILE)
    return PaintWindow::OnMouseDown(p);

#ifdef HAVE_VIBRATOR
  PlayHapticFeedback(HapticFeedbackType::PRESS);
#endif

  SetCapture();
  Invalidate();
  return true;
}

bool
TabMenuDisplay::OnMouseUp(PixelPoint p) noexcept
{
  if (down_tile == NO_TILE)
    return PaintWindow::OnMouseUp(p);

  /* like all XCSoar buttons: act on lift-off, inside the tile only */
  const unsigned tile = down_tile;
  OnCancelMode();
  if (FindTile(p) == tile)
    Open(tile);
  return true;
}

bool
TabMenuDisplay::OnMouseMove([[maybe_unused]] PixelPoint p,
                            [[maybe_unused]] unsigned keys) noexcept
{
  if (down_tile == NO_TILE)
    return false;

  Invalidate();
  return true;
}

bool
TabMenuDisplay::OnMouseWheel([[maybe_unused]] PixelPoint p,
                             int delta) noexcept
{
  SetScroll(int(scroll) - delta * metrics.pitch_y);
  return true;
}

void
TabMenuDisplay::OnCancelMode() noexcept
{
  if (down_tile != NO_TILE) {
    down_tile = NO_TILE;
    ReleaseCapture();
    Invalidate();
  }
  PaintWindow::OnCancelMode();
}

void
TabMenuDisplay::DrawTile(Canvas &canvas, unsigned i) const noexcept
{
  const Tile &tile = tiles[i];
  const PixelRect rc = GetTileRect(i);
  const bool pressed = i == down_tile;
  const bool focused = pressed || (i == cursor && HasCursorKeys() &&
                                   HasFocus());

  canvas.SetBackgroundTransparent();

  if (!look.lx) {
    canvas.DrawFilledRectangle(rc, look.list.GetBackgroundColor(
                                 i == cursor, focused, pressed));
    canvas.SetTextColor(look.list.GetTextColor(i == cursor, focused,
                                               pressed));
    canvas.Select(*look.button.font);
    const unsigned padding = Layout::GetTextPadding();
    canvas.DrawClippedText({rc.left + int(padding),
                            (rc.top + rc.bottom -
                             int(canvas.GetFontHeight())) / 2},
                           rc.GetWidth() - 2 * padding, tile.caption);
    return;
  }

  using namespace LX::Tile;
  DrawGradientBox(canvas, rc, focused ? FOCUS_TOP : TOP,
                  focused ? FOCUS_BOTTOM : BOTTOM,
                  focused ? FOCUS_BORDER : BORDER, LX::Scale(RADIUS));

  if (tile.icon.IsDefined())
    tile.icon.Draw(canvas, {(rc.left + rc.right) / 2,
                            rc.top + int(LX::Scale(ICON_CENTER_Y))},
                   LX::Scale(ICON_HEIGHT));

  canvas.SetTextColor(LX::TEXT);
  canvas.Select(label_font);
  const unsigned width = rc.GetWidth();
  const unsigned text_width = std::min(width, canvas.CalcTextWidth(tile.caption));
  canvas.DrawClippedText({rc.left + int(width - text_width) / 2,
                          rc.top + int(LX::Scale(LABEL_BASELINE)) -
                          int(label_font.GetAscentHeight())},
                         text_width, tile.caption);
}

void
TabMenuDisplay::DrawTitle(Canvas &canvas) const noexcept
{
  /* the tiles scroll underneath the title */
  const int rule_y = LX::Scale(LX::TITLE_RULE_Y);
  const int width = GetSize().width;
  canvas.DrawFilledRectangle({0, 0, width, metrics.top}, LX::BACKGROUND);
  canvas.DrawFilledRectangle({0, rule_y, width,
                              rule_y + std::max(1, int(LX::Scale(1)))},
                             LX::TEXT);

  canvas.SetBackgroundTransparent();
  canvas.SetTextColor(LX::TEXT);
  canvas.Select(title_font);
  canvas.DrawText({(width - int(canvas.CalcTextWidth(title))) / 2,
                   int(LX::Scale(LX::TITLE_BASELINE)) -
                   int(title_font.GetAscentHeight())}, title);
}

void
TabMenuDisplay::DrawScrollBar(Canvas &canvas) const noexcept
{
  const unsigned rows = (tiles.size() + metrics.columns - 1) / metrics.columns;
  const unsigned visible = GetVisibleRows();
  if (rows <= visible)
    return;

  using namespace LX::Tile;
  const int track_top = metrics.top;
  const int track = int(GetViewHeight()) - track_top;
  const int thumb = std::max(1, int(track * visible / rows));
  const int first_row = scroll / metrics.pitch_y;
  const int y = track_top + first_row * (track - thumb) / int(rows - visible);
  const int right = GetSize().width - LX::Scale(THUMB_MARGIN);
  canvas.DrawFilledRectangle({right - int(LX::Scale(THUMB_WIDTH)), y, right,
                              y + thumb}, THUMB);
}

void
TabMenuDisplay::OnPaint(Canvas &canvas) noexcept
{
  canvas.Clear(look.lx ? LX::BACKGROUND
                       : (look.dark_mode ? DarkColor(look.background_color)
                                         : COLOR_BLACK));

  for (unsigned i = 0; i < tiles.size(); ++i) {
    const PixelRect rc = GetTileRect(i);
    if (rc.top >= metrics.top && rc.bottom <= int(GetViewHeight()))
      DrawTile(canvas, i);
  }

  if (look.lx) {
    DrawTitle(canvas);
    DrawScrollBar(canvas);
  }
}

void
TabMenuDisplay::OnKillFocus() noexcept
{
  Invalidate();
  PaintWindow::OnKillFocus();
}

void
TabMenuDisplay::OnSetFocus() noexcept
{
  Invalidate();
  PaintWindow::OnSetFocus();
}
