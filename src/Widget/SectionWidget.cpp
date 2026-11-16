// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "SectionWidget.hpp"
#include "Look/DialogLook.hpp"
#include "Look/LXLook.hpp"
#include "ui/canvas/Canvas.hpp"
#include "ui/window/ContainerWindow.hpp"

/** The caption sits on the top edge; the box starts half way down. */
static unsigned
GetCaptionHeight(const DialogLook &look) noexcept
{
  return look.small_font.GetHeight();
}

void
SectionFrame::OnPaint(Canvas &canvas) noexcept
{
  using namespace LX::Section;

  const unsigned caption_height = GetCaptionHeight(look);
  PixelRect box = GetClientRect();
  box.top += caption_height / 2;
  box.left += 1;
  box.right -= 2;
  box.bottom -= 1;

  canvas.SelectWhitePen();
  canvas.SelectHollowBrush();
  canvas.DrawRoundRectangle(box, {2 * LX::Scale(RADIUS), 2 * LX::Scale(RADIUS)});

  /* the caption interrupts the outline */
  canvas.Select(look.small_font);
  canvas.SetTextColor(LX::TEXT);
  canvas.SetBackgroundColor(LX::BACKGROUND);
  canvas.SetBackgroundOpaque();
  canvas.DrawText({int(LX::Scale(CAPTION_X)), 0}, caption);
}

PixelSize
SectionWidget::GetFrameSize() const noexcept
{
  const unsigned padding = LX::Scale(LX::Section::PADDING);
  return {2 * padding, GetCaptionHeight(look) / 2 + 2 * padding +
          LX::Scale(LX::Section::GAP)};
}

PixelRect
SectionWidget::GetContentRect(const PixelRect &rc) const noexcept
{
  const int padding = LX::Scale(LX::Section::PADDING);
  return {rc.left + padding,
          rc.top + int(GetCaptionHeight(look) / 2) + padding,
          rc.right - padding,
          rc.bottom - padding - int(LX::Scale(LX::Section::GAP))};
}

PixelSize
SectionWidget::GetMinimumSize() const noexcept
{
  const PixelSize size = widget->GetMinimumSize(), frame_size = GetFrameSize();
  return {size.width + frame_size.width, size.height + frame_size.height};
}

PixelSize
SectionWidget::GetMaximumSize() const noexcept
{
  const PixelSize size = widget->GetMaximumSize(), frame_size = GetFrameSize();
  return {size.width + frame_size.width, size.height + frame_size.height};
}

void
SectionWidget::Initialise(ContainerWindow &parent,
                          const PixelRect &rc) noexcept
{
  widget->Initialise(parent, GetContentRect(rc));

  /* a window created later is below the earlier ones: the frame must
     not cover the hosted widget */
  WindowStyle style;
  style.Hide();
  frame.Create(parent, rc, style);
}

void
SectionWidget::Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept
{
  widget->Prepare(parent, GetContentRect(rc));
}

void
SectionWidget::Unprepare() noexcept
{
  widget->Unprepare();
}

bool
SectionWidget::Save(bool &changed) noexcept
{
  return widget->Save(changed);
}

bool
SectionWidget::Click() noexcept
{
  return widget->Click();
}

void
SectionWidget::ReClick() noexcept
{
  widget->ReClick();
}

void
SectionWidget::Show(const PixelRect &rc) noexcept
{
  frame.MoveAndShow(rc);
  widget->Show(GetContentRect(rc));
}

bool
SectionWidget::Leave() noexcept
{
  return widget->Leave();
}

void
SectionWidget::Hide() noexcept
{
  widget->Hide();
  frame.Hide();
}

void
SectionWidget::Move(const PixelRect &rc) noexcept
{
  frame.Move(rc);
  widget->Move(GetContentRect(rc));
}

bool
SectionWidget::SetFocus() noexcept
{
  return widget->SetFocus();
}

bool
SectionWidget::HasFocus() const noexcept
{
  return widget->HasFocus();
}

bool
SectionWidget::KeyPress(unsigned key_code) noexcept
{
  return widget->KeyPress(key_code);
}
