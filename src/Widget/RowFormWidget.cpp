// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "RowFormWidget.hpp"
#include "Form/Panel.hpp"
#include "Form/Button.hpp"
#include "Form/HLine.hpp"
#include "Look/DialogLook.hpp"
#include "Look/LXLook.hpp"
#include "Form/LXFieldGeometry.hpp"
#include "Dialogs/DialogSettings.hpp"
#include "UIGlobals.hpp"
#include "Screen/Layout.hpp"
#include "ui/control/LargeTextWindow.hpp"
#include "ui/canvas/Font.hpp"

#include <algorithm>
#include <cassert>

[[gnu::pure]]
static unsigned
GetMinimumHeight(const WndProperty &control, const DialogLook &look,
                 bool vertical) noexcept
{
  if (look.lx)
    return LX::FieldGeometry::FromLook().GetHeight(
      vertical && control.HasCaption() && !control.IsCheckBox(),
      look.small_font.GetHeight(), Layout::GetMinimumControlHeight());

  const unsigned padding = Layout::GetTextPadding();
  unsigned height = look.text_font.GetHeight();
  if (vertical && control.HasCaption())
    height *= 2;
  height += padding * 2;

  if (!control.IsReadOnly() && height < Layout::GetMinimumControlHeight())
    height = Layout::GetMinimumControlHeight();

  return height;
}

[[gnu::pure]]
static unsigned
GetMaximumHeight(const WndProperty &control, const DialogLook &look,
                 bool vertical) noexcept
{
  unsigned height = GetMinimumHeight(control, look, vertical);
  if (look.lx)
    return height;

  if (!control.IsReadOnly() && height < Layout::GetMaximumControlHeight())
    height = Layout::GetMaximumControlHeight();

  return height;
}

unsigned
RowFormWidget::Row::GetMinimumHeight(const DialogLook &look,
                                     bool vertical) const noexcept
{
  switch (type) {
  case Type::DUMMY:
    return 0;

  case Type::WIDGET: {
    const unsigned height = widget->GetMinimumSize().height;
    return height > 0 ? height : Layout::GetMinimumControlHeight();
  }

  case Type::GENERIC:
    break;

  case Type::EDIT:
    return ::GetMinimumHeight(GetControl(), look, vertical);

  case Type::BUTTON:
    return Layout::GetMinimumControlHeight();

  case Type::MULTI_LINE:
    return Layout::GetMinimumControlHeight();

  case Type::REMAINING:
    return Layout::GetMinimumControlHeight();
  }

  /* Use the window rectangle (outer size), not GetSize() (client
     area).  Move() sizes the outer window; feeding client height
     back into Move() shrinks bordered controls on every layout
     pass (e.g. VScrollPanel smooth scroll) until height reaches 0
     and PaintCanvas asserts. */
  return window->GetPosition().GetHeight();
}

unsigned
RowFormWidget::Row::GetMaximumHeight(const DialogLook &look,
                                     bool vertical) const noexcept
{
  switch (type) {
  case Type::DUMMY:
    return 0;

  case Type::WIDGET:
    return widget->GetMaximumSize().height;

  case Type::GENERIC:
    break;

  case Type::EDIT:
    return ::GetMaximumHeight(GetControl(), look, vertical);

  case Type::BUTTON:
    return Layout::GetMaximumControlHeight();

  case Type::MULTI_LINE:
    return Layout::GetMinimumControlHeight() * 3;

  case Type::REMAINING:
    return 4096;
  }

  return window->GetPosition().GetHeight();
}

/**
 * Does a value of this type fit into a column?  Text, files and
 * coordinates are too long for that.
 */
[[gnu::const]]
static bool
IsShortValue(DataField::Type type) noexcept
{
  switch (type) {
  case DataField::Type::BOOLEAN:
  case DataField::Type::INTEGER:
  case DataField::Type::REAL:
  case DataField::Type::ANGLE:
  case DataField::Type::ENUM:
  case DataField::Type::ROUGH_TIME:
  case DataField::Type::TIME:
  case DataField::Type::PREFIX:
  case DataField::Type::DATE:
    return true;

  case DataField::Type::STRING:
  case DataField::Type::FILE:
  case DataField::Type::MULTI_FILE:
  case DataField::Type::GEOPOINT:
    break;
  }

  return false;
}

bool
RowFormWidget::Row::IsShareable() const noexcept
{
  if (type != Type::EDIT)
    return false;

  const WndProperty &control = GetControl();
  const DataField *data_field = control.GetDataField();
  return control.HasCaption() && data_field != nullptr &&
    IsShortValue(data_field->GetType());
}

bool
RowFormWidget::Row::FitsCell(unsigned cell_width) const noexcept
{
  const WndProperty &control = GetControl();
  if (!control.IsCheckBox())
    return true;

  /* a check box shows its caption right of the box */
  const unsigned box = LX::Scale(LX::CheckBox::BOX_SIZE) +
    LX::Scale(LX::CheckBox::PADDING) + LX::Scale(LX::CheckBox::CAPTION_GAP);
  return control.GetRecommendedCaptionWidth() + box <= cell_width;
}

inline void
RowFormWidget::Row::UpdateLayout(ContainerWindow &parent,
                                 const PixelRect &_position,
                                 int caption_width) noexcept
{
  assert(type != Type::DUMMY);

  position = _position;

  if (type == Type::WIDGET) {
    Widget &widget = GetWidget();

    if (shown)
      widget.Move(position);
  } else {
    Window &window = GetWindow();

    if (type == Type::EDIT && GetControl().HasCaption())
      GetControl().SetCaptionWidth(caption_width);

    /* finally move and resize */
    window.Move(position);
  }

  if (visible)
    Show(parent);
}

inline void
RowFormWidget::Row::SetVisible(ContainerWindow &parent, bool _visible) noexcept
{
  if (_visible == visible)
    return;

  visible = _visible;
  if (!visible)
    Hide();
  else if (IsAvailable(UIGlobals::GetDialogSettings().expert))
    Show(parent);
}

void
RowFormWidget::Row::Show(ContainerWindow &parent) noexcept
{
  if (type == Type::WIDGET) {
    if (!initialised) {
      initialised = true;
      widget->Initialise(parent, position);
    }

    if (!prepared) {
      prepared = true;
      widget->Prepare(parent, position);
    }

    if (!shown) {
      shown = true;
      widget->Show(position);
    }
  } else if (type != Type::DUMMY)
    window->Show();
}

void
RowFormWidget::Row::Hide() noexcept
{
  if (type == Type::WIDGET) {
    if (shown) {
      shown = false;
      widget->Hide();
    }
  } else if (type != Type::DUMMY)
    window->Hide();
}

bool
RowFormWidget::Row::HasFocus() const noexcept
{
  switch (type) {
  case Type::DUMMY:
    break;

  case Type::WIDGET:
    return widget->HasFocus();

  case Type::GENERIC:
  case Type::EDIT:
  case Type::MULTI_LINE:
  case Type::BUTTON:
  case Type::REMAINING:
    return window->HasFocus();
  }

  return false;
}

RowFormWidget::RowFormWidget(const DialogLook &_look, bool _vertical) noexcept
  :look(_look), vertical(_vertical)
{
}

RowFormWidget::~RowFormWidget() noexcept
{
  rows.clear();

  if (IsDefined())
    DeleteWindow();
}

void
RowFormWidget::SetRowAvailable(unsigned i, bool available) noexcept
{
  Row &row = rows[i];
  if (available == row.available)
    return;

  row.available = available;
  UpdateLayout();

  /* The scroll panel sized this form while the row was hidden.
     Ask it to measure again, or the new rows stay inside the old
     short rectangle. */
  if (ContainerWindow *parent = GetWindow().GetParent())
    parent->OnChildContentHeightChanged();
}

void
RowFormWidget::SetRowVisible(unsigned i, bool visible) noexcept
{
  rows[i].SetVisible((ContainerWindow &)GetWindow(), visible);
}

void
RowFormWidget::SetExpertRow(unsigned i) noexcept
{
  Row &row = rows[i];
  assert(!row.expert);
  row.expert = true;
}

Window &
RowFormWidget::Add(Row::Type type, std::unique_ptr<Window> window) noexcept
{
  assert(IsDefined());
  assert(window->GetParent() == &GetWindow());
  assert(window->IsVisible());
  /* cannot append rows after a REMAINING row */
  assert(rows.empty() || rows.back().type != Row::Type::REMAINING);

  rows.emplace_back(type, std::move(window));
  return *rows.back().window;
}

void
RowFormWidget::AddSpacer() noexcept
{
  assert(IsDefined());

  auto window = std::make_unique<HLine>(GetLook());
  ContainerWindow &panel = (ContainerWindow &)GetWindow();
  const PixelRect rc = InitialControlRect(Layout::Scale(3));
  window->Create(panel, rc);
  Add(std::move(window));
}

void
RowFormWidget::AddMultiLine(const char *text) noexcept
{
  assert(IsDefined());

  const PixelRect rc =
    InitialControlRect(Layout::GetMinimumControlHeight());

  LargeTextWindowStyle style;
  style.SunkenEdge();

  ContainerWindow &panel = (ContainerWindow &)GetWindow();
  auto ltw = std::make_unique<LargeTextWindow>();
  ltw->Create(panel, rc, style);
  ltw->SetFont(look.text_font);
  ltw->SetColors(look.ReadOnlyValueBackground(), look.list.text_color,
                 look.ReadOnlyValueBorderColor());

  if (text != nullptr)
    ltw->SetText(text);

  Add(Row::Type::MULTI_LINE, std::move(ltw));
}

Button *
RowFormWidget::AddButton(const char *label,
                         std::function<void()> callback) noexcept
{
  assert(IsDefined());

  const PixelRect button_rc =
    InitialControlRect(Layout::GetMinimumControlHeight());

  WindowStyle button_style;
  button_style.TabStop();

  ContainerWindow &panel = (ContainerWindow &)GetWindow();

  return (Button *)&Add(Row::Type::BUTTON,
                        std::make_unique<Button>(panel, look.button, label,
                                                 button_rc, button_style,
                                                 std::move(callback)));
}

void
RowFormWidget::SetMultiLineText(unsigned i, const char *text) noexcept
{
  assert(text != nullptr);
  assert(rows[i].type == Row::Type::MULTI_LINE);

  auto &ltw = (LargeTextWindow &)*rows[i].window;
  ltw.SetText(text);
}

unsigned
RowFormWidget::GetRecommendedCaptionWidth() const noexcept
{
  const bool expert = UIGlobals::GetDialogSettings().expert;

  unsigned w = 0;
  for (const auto &i : rows) {
    if (!i.IsAvailable(expert))
      continue;

    if (i.type == Row::Type::EDIT) {
      unsigned x = i.GetControl().GetRecommendedCaptionWidth();
      if (x > w)
        w = x;
    }
  }

  return w;
}

struct RowFormWidget::Plan {
  struct Line {
    unsigned min_height = 0, max_height = 0;

    /** Does the line grow when there is excess screen space? */
    bool elastic = false;
  };

  unsigned columns = 1;

  /** The space left and right of the rows, and between columns. */
  unsigned margin = 0, gap = 0;

  /** One entry per row; a row which is not available spans no column. */
  boost::container::static_vector<RowFlow::Slot, 32u> slots;

  /**
   * One entry per row: the width reserved for its caption, or
   * negative if the caption is above the value.
   */
  boost::container::static_vector<int, 32u> caption_widths;

  boost::container::static_vector<Line, 32u> lines;

  unsigned GetMinimumHeight() const noexcept {
    unsigned height = 0;
    for (const auto &line : lines)
      height += line.min_height;
    return height;
  }

  unsigned GetMaximumHeight() const noexcept {
    unsigned height = 0;
    for (const auto &line : lines)
      height += line.max_height;
    return height;
  }
};

RowFormWidget::Plan
RowFormWidget::MakePlan(unsigned width) const noexcept
{
  const bool expert = UIGlobals::GetDialogSettings().expert;

  Plan plan;
  if (look.lx) {
    plan.margin = LX::Scale(LX::Form::MARGIN);
    plan.gap = LX::Scale(LX::Form::COLUMN_GAP);

    /* side by side captions need the space above the value, and
       a form that asks for them keeps them there */
    if (!vertical)
      plan.columns = RowFlow::CountColumns(width, plan.margin, plan.gap,
                                           LX::Scale(LX::Form::MIN_CELL_WIDTH));
  }

  /* put the available rows into lines */
  const unsigned cell_width = plan.columns > 1
    ? (width - 2 * plan.margin - plan.gap * (plan.columns - 1)) / plan.columns
    : width;

  boost::container::static_vector<bool, 32u> shareable;
  for (const auto &row : rows)
    if (row.IsAvailable(expert))
      shareable.push_back(plan.columns > 1 && row.IsShareable() &&
                          row.FitsCell(cell_width));

  boost::container::static_vector<RowFlow::Slot, 32u> assigned(shareable.size());
  const unsigned n_lines = RowFlow::Assign(shareable, plan.columns, assigned);
  plan.lines.resize(n_lines);

  auto next = assigned.begin();
  for (const auto &row : rows) {
    if (!row.IsAvailable(expert)) {
      plan.slots.push_back({0, 0, 0});
      continue;
    }

    const RowFlow::Slot slot = *next++;
    plan.slots.push_back(slot);

    /* the rows of a shared line have the caption above the value,
       and in the LX look so has a short row that is left over */
    const bool caption_above = vertical || slot.span < plan.columns ||
      (look.lx && row.IsShareable());
    auto &line = plan.lines[slot.line];
    line.min_height = std::max(line.min_height,
                               row.GetMinimumHeight(look, caption_above));
    line.max_height = std::max(line.max_height,
                               row.GetMaximumHeight(look, caption_above));
    line.elastic |= row.IsElastic(look, caption_above);
  }

  /* The captions left of the values are as wide as the widest one. */
  int caption_width = -1;
  const auto caption_left = [&](std::size_t i) {
    return !vertical && plan.slots[i].span == plan.columns &&
      !(look.lx && rows[i].IsShareable());
  };

  for (std::size_t i = 0; i < rows.size(); ++i)
    if (rows[i].type == Row::Type::EDIT && rows[i].IsAvailable(expert) &&
        caption_left(i))
      caption_width = std::max<int>(caption_width,
        rows[i].GetControl().GetRecommendedCaptionWidth());

  if (caption_width * 3 > int(width * 2))
    caption_width = width * 2 / 3;

  for (std::size_t i = 0; i < rows.size(); ++i)
    plan.caption_widths.push_back(caption_left(i) ? caption_width : -1);

  return plan;
}

unsigned
RowFormWidget::GetCurrentWidth() const noexcept
{
  return IsDefined() ? GetWindow().GetClientRect().GetWidth() : 0u;
}

void
RowFormWidget::UpdateLayout() noexcept
{
  const PixelRect client_rect = GetWindow().GetClientRect();
  const unsigned total_width = client_rect.GetWidth();
  const unsigned total_height = client_rect.GetHeight();

  const bool expert = UIGlobals::GetDialogSettings().expert;

  const Plan plan = MakePlan(total_width);

  /* count the number of "elastic" lines and determine the minimum
     total height */
  const unsigned min_height = plan.GetMinimumHeight();
  unsigned n_elastic = 0;
  for (const auto &line : plan.lines)
    if (line.elastic)
      ++n_elastic;

  /* how much excess height in addition to the minimum height? */
  unsigned excess_height = min_height < total_height
    ? total_height - min_height
    : 0;

  /* determine the lines' vertical extent */
  boost::container::static_vector<int, 33u> line_tops;
  line_tops.push_back(client_rect.top);
  for (const auto &line : plan.lines) {
    unsigned height = line.min_height;
    if (excess_height > 0 && line.elastic) {
      assert(n_elastic > 0);

      /* distribute excess height among all elastic lines */
      unsigned grow_height = excess_height / n_elastic;
      if (grow_height > 0) {
        height += grow_height;
        if (height > line.max_height) {
          /* never grow beyond declared maximum height */
          grow_height = line.max_height - height + grow_height;
          height = line.max_height;
        }

        excess_height -= grow_height;
      }

      --n_elastic;
    }

    /* a row is at least one pixel high */
    line_tops.push_back(line_tops.back() + std::max(height, 1u));
  }

  assert(excess_height == 0 || n_elastic == 0);

  /* now move and resize the rows */
  for (std::size_t i = 0; i < rows.size(); ++i) {
    Row &row = rows[i];
    if (!row.IsAvailable(expert)) {
      row.Hide();
      continue;
    }

    const RowFlow::Slot &slot = plan.slots[i];
    const auto extent = RowFlow::GetExtent(slot, plan.columns, total_width,
                                           plan.margin, plan.gap);
    const PixelRect rc{client_rect.left + extent.left, line_tops[slot.line],
                       client_rect.left + extent.right,
                       line_tops[slot.line + 1]};
    row.UpdateLayout((ContainerWindow &)GetWindow(), rc,
                     plan.caption_widths[i]);
  }
}

PixelSize
RowFormWidget::GetMinimumSize() const noexcept
{
  const unsigned value_width =
    look.text_font.TextSize("Foo Bar Foo Bar").width;

  const bool expert = UIGlobals::GetDialogSettings().expert;

  const unsigned edit_width = vertical
    ? std::max(GetRecommendedCaptionWidth(), value_width)
    : (GetRecommendedCaptionWidth() + value_width);

  PixelSize size(edit_width, MakePlan(GetCurrentWidth()).GetMinimumHeight());
  for (const auto &i : rows) {
    if (i.IsAvailable(expert) && i.type == Row::Type::WIDGET) {
      const unsigned width = i.widget->GetMinimumSize().width;
      if (width > size.width)
        size.width = width;
    }
  }

  return size;
}

PixelSize
RowFormWidget::GetMaximumSize() const noexcept
{
  const unsigned value_width =
    look.text_font.TextSize("Foo Bar Foo Bar").width * 2;

  const bool expert = UIGlobals::GetDialogSettings().expert;

  const unsigned edit_width = vertical
    ? std::max(GetRecommendedCaptionWidth(), value_width)
    : (GetRecommendedCaptionWidth() + value_width);

  PixelSize size(edit_width, MakePlan(GetCurrentWidth()).GetMaximumHeight());
  for (const auto &i : rows) {
    if (i.IsAvailable(expert) && i.type == Row::Type::WIDGET) {
      const unsigned width = i.widget->GetMaximumSize().width;
      if (width > size.width)
        size.width = width;
    }
  }

  return size;
}

void
RowFormWidget::Initialise(ContainerWindow &parent,
                          const PixelRect &rc) noexcept
{
  assert(!IsDefined());
  assert(rows.empty());

  WindowStyle style;
  style.Hide();
  style.ControlParent();

  SetWindow(std::make_unique<PanelControl>(parent, rc, style));
}

void
RowFormWidget::Unprepare() noexcept
{
  for (auto &i : rows)
    i.Unprepare();

  WindowWidget::Unprepare();
}

void
RowFormWidget::Show(const PixelRect &rc) noexcept
{
  PixelRect safe_rc = rc;
  if (safe_rc.GetHeight() == 0)
    safe_rc.bottom = safe_rc.top + 1;

  Window &panel = GetWindow();
  panel.Move(safe_rc);

  UpdateLayout();

  panel.Show();
}

void
RowFormWidget::Move(const PixelRect &rc) noexcept
{
  WindowWidget::Move(rc);

  UpdateLayout();
}

bool
RowFormWidget::SetFocus() noexcept
{
  if (rows.empty())
    return false;

  ContainerWindow &panel = (ContainerWindow &)GetWindow();
  return panel.FocusFirstControl();
}

bool
RowFormWidget::HasFocus() const noexcept
{
  return std::any_of(rows.begin(), rows.end(), [](const auto &r){
    return r.HasFocus();
  });
}

bool
RowFormWidget::FocusAdjacent(bool next) noexcept
{
  int current = -1;
  for (unsigned i = 0; i < rows.size(); ++i) {
    if (rows[i].HasFocus()) {
      current = (int)i;
      break;
    }
  }
  if (current < 0)
    return false;

  const int step = next ? 1 : -1;
  for (int i = current + step;
       i >= 0 && i < (int)rows.size(); i += step) {
    const Row &row = rows[i];
    if (row.type == Row::Type::DUMMY || row.type == Row::Type::WIDGET)
      continue;

    Window &window = rows[i].GetWindow();
    if (!window.IsVisible() || !window.IsEnabled() || !window.IsTabStop())
      continue;

    window.SetFocus();
    return true;
  }

  return false;
}
