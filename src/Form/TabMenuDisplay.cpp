// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "TabMenuDisplay.hpp"
#include "TabMenuData.hpp"
#include "Widget/PagerWidget.hpp"
#include "Widget/VScrollWidget.hpp"
#include "Screen/Layout.hpp"
#include "Form/Button.hpp"
#include "ui/event/KeyCode.hpp"
#include "ui/canvas/Canvas.hpp"
#include "ui/canvas/Icon.hpp"
#include "Look/DialogLook.hpp"
#include "Language/Language.hpp"
#include "util/StringFormat.hpp"
#include "Asset.hpp"

#include <algorithm>
#include <cassert>

void
TabMenuDisplay::MainMenuButton::Draw(Canvas &canvas, const DialogLook &look,
                                     bool focused, bool pressed,
                                     bool selected) const noexcept
{
  canvas.DrawFilledRectangle(rc,
                             look.list.GetBackgroundColor(selected, focused,
                                                          pressed));
  canvas.SetTextColor(look.list.GetTextColor(selected, focused, pressed));
  canvas.SetBackgroundTransparent();
  canvas.Select(*look.button.font);

  const unsigned padding = Layout::GetTextPadding();
  const unsigned icon_height = std::min(rc.GetHeight() / 2,
                                        look.button.font->GetHeight());
  const int icon_x = rc.left + padding + icon_height / 2;
  if (icon != nullptr && icon_height > 0)
    icon->Draw(canvas, {icon_x, (rc.top + rc.bottom) / 2}, icon_height);

  const int text_x = icon != nullptr
    ? icon_x + int(icon_height / 2 + padding) : rc.left + int(padding);
  const int width = std::max(0, rc.right - text_x - int(padding));
  const int y = (rc.top + rc.bottom - int(canvas.GetFontHeight())) / 2;
  canvas.DrawClippedText({text_x, y}, width, caption);
}

TabMenuDisplay::TabMenuDisplay(PagerWidget &_pager,
                               const DialogLook &_look) noexcept
  :pager(_pager), look(_look)
{
}

void
TabMenuDisplay::InitMenu(const TabMenuGroup groups[],
                         unsigned n_groups) noexcept
{
  assert(groups != nullptr);
  assert(n_groups > 0);
  assert(n_groups <= MAX_MAIN_MENU_ITEMS);

  for (unsigned i = 0; i < n_groups; i++) {
    const auto &g = groups[i];
    const unsigned first_page_index = buttons.size();
    const unsigned main_menu_index = main_menu_buttons.size();

    for (auto p = g.pages; p->Load != nullptr; ++p) {
      if (p->IsAvailable != nullptr && !p->IsAvailable())
        continue;

      auto &page_button = buttons.append();
      page_button.main_menu_index = main_menu_index;
      page_button.caption = gettext(p->menu_caption);

      /* Wrap panel in VScrollWidget for automatic scrolling when needed */
      auto panel = p->Load();
      auto scroll_panel = std::make_unique<VScrollWidget>(std::move(panel), look);
      pager.Add(std::move(scroll_panel));
    }

    if (buttons.size() == first_page_index)
      continue;

    auto &mb = main_menu_buttons.append();
    mb.caption = gettext(g.caption);
    mb.first_page_index = first_page_index;
    mb.last_page_index = buttons.size() - 1;
  }
}

const char *
TabMenuDisplay::GetCaption(char buffer[], size_t size) const noexcept
{
  const unsigned page = pager.GetCurrentIndex();
  if (page >= PAGE_OFFSET) {
    const unsigned i = page - PAGE_OFFSET;
    StringFormat(buffer, size, "%s > %s",
                 gettext(GetPageParentCaption(i)),
                 buttons[i].caption);
    return buffer;
  } else
    return nullptr;
}

int
TabMenuDisplay::GetPageNum(MenuTabIndex i) const noexcept
{
  assert(i.IsSub());

  assert(i.main_index < main_menu_buttons.size());
  assert(i.sub_index < GetNumPages());

  const MainMenuButton &main_button = GetMainMenuButton(i.main_index);
  return main_button.first_page_index + i.sub_index;
}

static unsigned
GetTabLineHeight() noexcept
{
  return Layout::Scale(2);
}

void
TabMenuDisplay::UpdateLayout() noexcept
{
  const auto window_size = GetSize();
  const unsigned gap = GetTabLineHeight();
  const unsigned min_width = Layout::GetMinimumControlHeight() * 3;
  main_columns = window_size.width >= min_width * 3 ? 3 : 2;
  sub_columns = window_size.width >= min_width * 2 ? 2 : 1;
  const unsigned main_rows = (GetNumMainMenuItems() + main_columns - 1) /
    main_columns;
  unsigned max_sub_rows = 1;
  for (const auto &main : main_menu_buttons)
    max_sub_rows = std::max(max_sub_rows,
                            (main.NumSubMenus() + sub_columns - 1) /
                            sub_columns);

  const unsigned rows = main_rows + max_sub_rows;
  const unsigned spacing = gap * (rows + 1);
  const unsigned available = window_size.height > spacing
    ? window_size.height - spacing : window_size.height;
  const unsigned height = std::max(1u,
    std::min(Layout::GetMaximumControlHeight(), available / rows));
  const unsigned main_width = window_size.width / main_columns;
  const unsigned sub_width = window_size.width / sub_columns;
  const unsigned sub_top = gap + main_rows * (height + gap) + gap;

  for (unsigned main_i = 0; main_i < main_menu_buttons.size(); ++main_i) {
    MainMenuButton &main = main_menu_buttons[main_i];
    const unsigned column = main_i % main_columns;
    const unsigned row = main_i / main_columns;
    main.rc.left = column * main_width + gap;
    main.rc.right = (column + 1) * main_width - gap;
    main.rc.top = gap + row * (height + gap);
    main.rc.bottom = main.rc.top + height;
    main.renderer.InvalidateLayout();
    for (unsigned page_i = main.first_page_index;
         page_i <= main.last_page_index; ++page_i) {
      SubMenuButton &page = buttons[page_i];
      const unsigned i = page_i - main.first_page_index;
      page.rc.left = (i % sub_columns) * sub_width + gap;
      page.rc.right = (i % sub_columns + 1) * sub_width - gap;
      page.rc.top = sub_top + (i / sub_columns) * (height + gap);
      page.rc.bottom = page.rc.top + height;
      page.renderer.InvalidateLayout();
    }
  }
}

inline const PixelRect &
TabMenuDisplay::GetButtonPosition(MenuTabIndex i) const noexcept
{
  assert(!i.IsNone());

  return i.IsMain()
    ? GetMainMenuButtonSize(i.main_index)
    : GetSubMenuButtonSize(GetPageNum(i));
}

TabMenuDisplay::MenuTabIndex
TabMenuDisplay::IsPointOverButton(PixelPoint Pos,
                                  unsigned mainIndex) const noexcept
{
  // scan main menu buttons
  for (unsigned i = 0; i < GetNumMainMenuItems(); i++)
    if (GetMainMenuButtonSize(i).Contains(Pos))
      return MenuTabIndex(i);


  // scan visible submenu
  if (mainIndex < GetNumMainMenuItems()) {
    const MainMenuButton &main_button = GetMainMenuButton(mainIndex);
    for (unsigned i = main_button.first_page_index;
         i <= main_button.last_page_index; ++i) {
      if (GetSubMenuButtonSize(i).Contains(Pos))
        return MenuTabIndex(mainIndex, i - main_button.first_page_index);
    }
  }

  return MenuTabIndex::None();
}

void
TabMenuDisplay::OnPageFlipped() noexcept
{
  const unsigned i = pager.GetCurrentIndex();
  if (i >= PAGE_OFFSET) {
    SetCursor(i - PAGE_OFFSET);
    selecting_group = false;
  }
}

void
TabMenuDisplay::SetCursor(unsigned i) noexcept
{
  if (GetNumPages() == 0)
    return;

  i = std::min(i, GetNumPages() - 1);
  if (i == cursor)
    return;

  if (IsDefined()) {
    if (SupportsPartialRedraw() &&
        GetPageMainIndex(cursor) == GetPageMainIndex(i)) {
      Invalidate(GetSubMenuButtonSize(cursor));
      Invalidate(GetSubMenuButtonSize(i));
    } else
      Invalidate();
  }

  cursor = i;
}

void
TabMenuDisplay::OnResize(PixelSize new_size) noexcept
{
  PaintWindow::OnResize(new_size);
  UpdateLayout();
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
    return !selecting_group || GetPageMainIndex(cursor) >= main_columns;

  case KEY_DOWN:
    return selecting_group ||
      cursor + sub_columns <=
      main_menu_buttons[GetPageMainIndex(cursor)].last_page_index;

  default:
    return false;
  }
}

bool
TabMenuDisplay::OnKeyDown(unsigned key_code) noexcept
{
  switch (key_code) {
  case KEY_RETURN:
    if (selecting_group) {
      selecting_group = false;
      Invalidate();
    } else
      pager.ClickPage(PAGE_OFFSET + cursor);
    return true;

  case KEY_RIGHT: case KEY_LEFT: case KEY_UP: case KEY_DOWN: {
    const unsigned group = GetPageMainIndex(cursor);
    const unsigned column = selecting_group
      ? group % main_columns
      : (cursor - main_menu_buttons[group].first_page_index) % sub_columns;
    const unsigned columns = selecting_group ? main_columns : sub_columns;
    if ((key_code == KEY_LEFT && column == 0) ||
        (key_code == KEY_RIGHT && column + 1 == columns))
      return true;
    const int step = key_code == KEY_LEFT ? -1 : key_code == KEY_RIGHT ? 1 :
      key_code == KEY_UP ? -int(selecting_group ? main_columns : sub_columns) :
      int(selecting_group ? main_columns : sub_columns);
    if (selecting_group) {
      const int next = int(group) + step;
      if (next >= 0 && next < int(GetNumMainMenuItems()))
        SetCursor(main_menu_buttons[next].first_page_index);
      else if (key_code == KEY_DOWN) {
        selecting_group = false;
        Invalidate();
      }
    } else {
      const auto &main = main_menu_buttons[group];
      const int next = int(cursor) + step;
      if (next >= int(main.first_page_index) &&
          next <= int(main.last_page_index))
        SetCursor(next);
      else if (key_code == KEY_UP && next < int(main.first_page_index)) {
        selecting_group = true;
        Invalidate();
      }
    }
    return true;
  }

  default:
    return false;
  }
}

bool
TabMenuDisplay::OnMouseDown(PixelPoint Pos) noexcept
{
  DragEnd();

  // If possible -> Give focus to the Control
  SetFocus();

  down_index = IsPointOverButton(Pos, GetPageMainIndex(cursor));

  if (!down_index.IsNone()) {
#ifdef HAVE_VIBRATOR
    PlayHapticFeedback(HapticFeedbackType::PRESS);
#endif

    dragging = true;
    SetCapture();

    InvalidateButton(down_index);
    return true;
  }
  return PaintWindow::OnMouseDown(Pos);
}

bool
TabMenuDisplay::OnMouseUp(PixelPoint Pos) noexcept
{
  if (dragging) {
    DragEnd();

    const MenuTabIndex di = IsPointOverButton(Pos, GetPageMainIndex(cursor));

    if (di == down_index) {

      // sub menu click
      if (di.IsSub())
        pager.ClickPage(PAGE_OFFSET + GetPageNum(di));

      // main menu click
      else if (di.IsMain()) {
        /* move cursor to first item in this menu */
        cursor = main_menu_buttons[di.main_index].first_page_index;
        selecting_group = false;
        Invalidate();
      } else {
        InvalidateButton(down_index);
      }
    }

    down_index = MenuTabIndex::None();

    return true;
  } else {
    return PaintWindow::OnMouseUp(Pos);
  }
}

bool
TabMenuDisplay::OnMouseMove(PixelPoint p,
                            [[maybe_unused]] unsigned keys) noexcept
{
  if (down_index.IsNone())
    return false;

  const PixelRect &rc = GetButtonPosition(down_index);
  const bool tmp = !rc.Contains(p);
  if (drag_off_button != tmp) {
    drag_off_button = tmp;
    Invalidate(rc);
  }
  return true;
}

inline void
TabMenuDisplay::PaintMainMenuBorder(Canvas &canvas) const noexcept
{
  PixelRect rc = GetMainMenuButtonSize(0);
  rc.right = GetSize().width;
  rc.bottom = GetMainMenuButtonSize(GetNumMainMenuItems() - 1).bottom;
  rc.Grow(GetTabLineHeight());

  canvas.DrawFilledRectangle(rc, look.dark_mode
                             ? DarkColor(look.background_color)
                             : COLOR_BLACK);
}

inline void
TabMenuDisplay::PaintMainMenuItems(Canvas &canvas) const noexcept
{
  PaintMainMenuBorder(canvas);

  const bool is_focused = !HasCursorKeys() || HasFocus();

  unsigned main_menu_index = 0;
  for (auto i = main_menu_buttons.begin(),
         end = main_menu_buttons.end(); i != end;
       ++i, ++main_menu_index) {
    const bool isDown = main_menu_index == down_index.main_index &&
      !down_index.IsSub() && !drag_off_button;

    const bool is_selected = isDown ||
      main_menu_index == GetPageMainIndex(cursor);

    main_menu_buttons[main_menu_index].Draw(canvas, look,
      is_focused && selecting_group, isDown, is_selected);
  }
}

inline void
TabMenuDisplay::PaintSubMenuBorder(Canvas &canvas,
                                   const MainMenuButton &main_button) const noexcept
{
  PixelRect rc = GetSubMenuButtonSize(main_button.first_page_index);
  rc.right = GetSize().width;
  rc.bottom = GetSubMenuButtonSize(main_button.last_page_index).bottom;
  rc.Grow(GetTabLineHeight());

  canvas.DrawFilledRectangle(rc, look.dark_mode
                             ? DarkColor(look.background_color)
                             : COLOR_BLACK);
}

inline void
TabMenuDisplay::PaintSubMenuItems(Canvas &canvas) const noexcept
{
  const MainMenuButton &main_button =
    GetMainMenuButton(GetPageMainIndex(cursor));

  PaintSubMenuBorder(canvas, main_button);

  assert(main_button.first_page_index < buttons.size());
  assert(main_button.last_page_index < buttons.size());

  const bool is_focused = !HasCursorKeys() || HasFocus();

  for (unsigned first_page_index = main_button.first_page_index,
         last_page_index = main_button.last_page_index,
         page_index = first_page_index;
       page_index <= last_page_index; ++page_index) {
    const unsigned sub_index = page_index - first_page_index;

    const bool is_pressed = sub_index == down_index.sub_index &&
      !drag_off_button;

    const bool is_cursor = page_index == cursor;
    const bool is_selected = is_pressed || is_cursor;

    buttons[page_index].Draw(canvas, look,
                             is_focused && !selecting_group,
                             is_pressed, is_selected);
  }
}

void
TabMenuDisplay::OnPaint(Canvas &canvas) noexcept
{
  PaintMainMenuItems(canvas);
  PaintSubMenuItems(canvas);
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

void
TabMenuDisplay::DragEnd() noexcept
{
  if (dragging) {
    dragging = false;
    drag_off_button = false;
    ReleaseCapture();
  }
}
