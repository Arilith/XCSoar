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

/** Does the item (or any tile below it) exist on this platform? */
static bool
IsAvailable(const TabMenuItem &item) noexcept
{
  if (item.IsAvailable != nullptr && !item.IsAvailable())
    return false;

  if (item.children == nullptr)
    return true;

  for (auto c = item.children; c->caption != nullptr; ++c)
    if (IsAvailable(*c))
      return true;

  return false;
}

void
TabMenuDisplay::InitMenu(const TabMenuItem *items,
                         const char *title) noexcept
{
  auto &root = nodes.append();
  root.caption = title;
  root.load = nullptr;
  root.action = nullptr;
  root.page = NONE;
  root.parent = NONE;
  root.first_child = root.num_children = root.focus = 0;

  Build(items, 0);
}

void
TabMenuDisplay::Build(const TabMenuItem *items, unsigned parent) noexcept
{
  /* the tiles of a grid are consecutive, so create them all before
     the grids below them */
  const unsigned first = nodes.size();
  for (auto i = items; i->caption != nullptr; ++i) {
    if (!IsAvailable(*i))
      continue;

    auto &node = nodes.append();
    node.caption = gettext(i->caption);
    node.load = i->Load;
    node.action = i->Action;
    node.page = NONE;
    node.parent = parent;
    node.first_child = node.num_children = node.focus = 0;
    LoadTileIcon(node.icon, i->icon);
  }

  nodes[parent].first_child = first;
  nodes[parent].num_children = nodes.size() - first;

  /* pages are added in the order of the tiles, depth first: the order
     in which they are saved */
  unsigned n = first;
  for (auto i = items; i->caption != nullptr; ++i) {
    if (!IsAvailable(*i))
      continue;

    const unsigned node = n++;
    if (i->Load != nullptr) {
      nodes[node].page = page_nodes.size();
      page_nodes.append(node);

      /* Wrap panel in VScrollWidget for automatic scrolling when needed */
      pager.Add(std::make_unique<VScrollWidget>(i->Load(), look));
    } else if (i->children != nullptr)
      Build(i->children, node);
  }
}

const char *
TabMenuDisplay::GetCaption(char buffer[], size_t size) const noexcept
{
  const unsigned page = pager.GetCurrentIndex();
  if (page < PAGE_OFFSET)
    return IsSubGrid() ? nodes[current].caption : nullptr;

  const Node &node = nodes[page_nodes[page - PAGE_OFFSET]];
  if (node.parent == 0)
    return node.caption;

  StringFormat(buffer, size, "%s > %s", nodes[node.parent].caption,
               node.caption);
  return buffer;
}

unsigned
TabMenuDisplay::FindPage(std::unique_ptr<Widget> (*load)()) const noexcept
{
  for (const unsigned node : page_nodes)
    if (nodes[node].load == load)
      return PAGE_OFFSET + nodes[node].page;

  assert(false);
  return PAGE_OFFSET;
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

unsigned
TabMenuDisplay::GetNumRows() const noexcept
{
  return (GetNumTiles() + metrics.columns - 1) / metrics.columns;
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
  for (unsigned i = 0; i < GetNumTiles(); ++i) {
    const PixelRect rc = GetTileRect(i);
    if (rc.top >= metrics.top && rc.bottom <= int(GetViewHeight()) &&
        rc.Contains(p))
      return i;
  }

  return NONE;
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
  const unsigned rows = GetNumRows();
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
TabMenuDisplay::SetCurrent(unsigned node) noexcept
{
  current = node;
  scroll = 0;

  if (IsDefined()) {
    UpdateLayout();
    EnsureVisible(GetCursorTile());
    Invalidate();
  }

  if (level_callback)
    level_callback();
}

void
TabMenuDisplay::EnterGrid(unsigned node) noexcept
{
  SetCurrent(node);
}

bool
TabMenuDisplay::LeaveSubGrid() noexcept
{
  if (!IsSubGrid())
    return false;

  SetCurrent(nodes[current].parent);
  return true;
}

void
TabMenuDisplay::OnPageFlipped() noexcept
{
  const unsigned i = pager.GetCurrentIndex();
  if (i < PAGE_OFFSET || i - PAGE_OFFSET >= page_nodes.size())
    return;

  /* show the grid of the page, with the tile of the page focused */
  const unsigned node = page_nodes[i - PAGE_OFFSET];
  const unsigned parent = nodes[node].parent;
  nodes[parent].focus = node - nodes[parent].first_child;
  if (parent != current)
    SetCurrent(parent);
  else if (IsDefined())
    EnsureVisible(GetCursorTile());
}

void
TabMenuDisplay::SetCursor(unsigned i) noexcept
{
  if (nodes[0].num_children == 0)
    return;

  nodes[0].focus = std::min<unsigned>(i, nodes[0].num_children - 1);
  if (IsDefined() && current == 0) {
    EnsureVisible(GetCursorTile());
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
TabMenuDisplay::Open(unsigned node) noexcept
{
  const Node &n = nodes[node];

  if (n.action != nullptr)
    n.action();
  else if (n.page != NONE)
    pager.ClickPage(PAGE_OFFSET + n.page);
  else if (n.num_children == 1)
    /* nothing to choose from */
    Open(n.first_child);
  else
    EnterGrid(node);
}

void
TabMenuDisplay::MoveCursor(int delta) noexcept
{
  const int next = int(GetCursorTile()) + delta;
  if (next >= 0 && next < int(GetNumTiles()))
    Cursor() = next;
  else if (delta > 0 && GetCursorTile() / metrics.columns + 1 < GetNumRows())
    /* moving down from a column that is missing in the last row */
    Cursor() = GetNumTiles() - 1;
  else
    return;

  EnsureVisible(GetCursorTile());
  Invalidate();
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
    return GetCursorTile() >= metrics.columns;

  case KEY_DOWN:
    return GetCursorTile() / metrics.columns + 1 < GetNumRows();

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
    Open(nodes[current].first_child + GetCursorTile());
    return true;

  case KEY_LEFT:
    if (GetCursorTile() % columns > 0)
      MoveCursor(-1);
    return true;

  case KEY_RIGHT:
    if (GetCursorTile() % columns + 1 < unsigned(columns))
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
  if (down_tile == NONE)
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
  if (down_tile == NONE)
    return PaintWindow::OnMouseUp(p);

  /* like all XCSoar buttons: act on lift-off, inside the tile only */
  const unsigned tile = down_tile;
  OnCancelMode();
  if (FindTile(p) == tile) {
    Cursor() = tile;
    Open(nodes[current].first_child + tile);
  }
  return true;
}

bool
TabMenuDisplay::OnMouseMove([[maybe_unused]] PixelPoint p,
                            [[maybe_unused]] unsigned keys) noexcept
{
  if (down_tile == NONE)
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
  if (down_tile != NONE) {
    down_tile = NONE;
    ReleaseCapture();
    Invalidate();
  }
  PaintWindow::OnCancelMode();
}

void
TabMenuDisplay::DrawTile(Canvas &canvas, unsigned i) const noexcept
{
  const Node &tile = GetTile(i);
  const PixelRect rc = GetTileRect(i);
  const bool pressed = i == down_tile;
  const bool is_cursor = i == GetCursorTile();
  const bool focused = pressed || (is_cursor && HasCursorKeys() &&
                                   HasFocus());

  canvas.SetBackgroundTransparent();

  if (!look.lx) {
    canvas.DrawFilledRectangle(rc, look.list.GetBackgroundColor(
                                 is_cursor, focused, pressed));
    canvas.SetTextColor(look.list.GetTextColor(is_cursor, focused, pressed));
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

  const char *title = nodes[current].caption;
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
  const unsigned rows = GetNumRows();
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

  for (unsigned i = 0; i < GetNumTiles(); ++i) {
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
