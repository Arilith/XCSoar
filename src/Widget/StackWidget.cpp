// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "StackWidget.hpp"

#include <algorithm>

void
StackWidget::Layout() noexcept
{
  for (unsigned pass = 0; pass < 2; ++pass) {
    int top = rc.top;
    for (auto &widget : widgets) {
      const int height = std::max(1u, widget->GetMinimumSize().height);
      widget->Move({rc.left, top, rc.right, top + height});
      top += height;
    }
  }
}

PixelSize
StackWidget::GetMinimumSize() const noexcept
{
  PixelSize size{0, 0};
  for (const auto &widget : widgets) {
    const PixelSize s = widget->GetMinimumSize();
    size.width = std::max(size.width, s.width);
    size.height += s.height;
  }

  return size;
}

PixelSize
StackWidget::GetMaximumSize() const noexcept
{
  /* never stretched beyond what the widgets need */
  return GetMinimumSize();
}

void
StackWidget::Initialise(ContainerWindow &parent, const PixelRect &_rc) noexcept
{
  rc = _rc;
  for (auto &widget : widgets)
    widget->Initialise(parent, rc);
}

void
StackWidget::Prepare(ContainerWindow &parent, const PixelRect &_rc) noexcept
{
  rc = _rc;
  for (auto &widget : widgets)
    widget->Prepare(parent, rc);
}

void
StackWidget::Unprepare() noexcept
{
  for (auto &widget : widgets)
    widget->Unprepare();
}

bool
StackWidget::Save(bool &changed) noexcept
{
  return std::all_of(widgets.begin(), widgets.end(), [&changed](auto &w){
    return w->Save(changed);
  });
}

bool
StackWidget::Click() noexcept
{
  return std::all_of(widgets.begin(), widgets.end(),
                     [](auto &w){ return w->Click(); });
}

void
StackWidget::ReClick() noexcept
{
  for (auto &widget : widgets)
    widget->ReClick();
}

void
StackWidget::Show(const PixelRect &_rc) noexcept
{
  rc = _rc;
  for (auto &widget : widgets)
    widget->Show(rc);
  Layout();
}

bool
StackWidget::Leave() noexcept
{
  return std::all_of(widgets.begin(), widgets.end(),
                     [](auto &w){ return w->Leave(); });
}

void
StackWidget::Hide() noexcept
{
  for (auto &widget : widgets)
    widget->Hide();
}

void
StackWidget::Move(const PixelRect &_rc) noexcept
{
  rc = _rc;
  Layout();
}

bool
StackWidget::SetFocus() noexcept
{
  return std::any_of(widgets.begin(), widgets.end(),
                     [](auto &w){ return w->SetFocus(); });
}

bool
StackWidget::HasFocus() const noexcept
{
  return std::any_of(widgets.begin(), widgets.end(),
                     [](const auto &w){ return w->HasFocus(); });
}

bool
StackWidget::KeyPress(unsigned key_code) noexcept
{
  return std::any_of(widgets.begin(), widgets.end(),
                     [key_code](auto &w){ return w->KeyPress(key_code); });
}
