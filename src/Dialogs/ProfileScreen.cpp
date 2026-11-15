// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "ProfileScreen.hpp"
#include "Form/DataField/File.hpp"
#include "Language/Language.hpp"
#include "Look/FontDescription.hpp"
#include "Look/LXLook.hpp"
#include "Profile/File.hpp"
#include "Profile/Map.hpp"
#include "Profile/ProfileSummary.hpp"
#include "ProfilePasswordDialog.hpp"
#include "Renderer/GradientRenderer.hpp"
#include "Renderer/PeopleIconRenderer.hpp"
#include "Renderer/SoftButtonRenderer.hpp"
#include "Screen/Layout.hpp"
#include "system/Path.hpp"
#include "ui/canvas/Canvas.hpp"
#include "util/StringCompare.hxx"

#include <algorithm>

/** The profile file name without the ".prf" suffix. */
static std::string
GetProfileName(Path path) noexcept
{
  std::string name{path.GetBase().c_str()};

  constexpr std::string_view suffix = ".prf";
  if (name.size() > suffix.size() &&
      StringIsEqualIgnoreCase(name.c_str() + name.size() - suffix.size(),
                              suffix.data()))
    name.resize(name.size() - suffix.size());

  return name;
}

static std::vector<std::string>
LoadSummary(Path path) noexcept
{
  try {
    /* do not reveal the contents of a protected profile */
    if (ProfileFileHasPassword(path) != TriState::FALSE)
      return {};

    ProfileMap map;
    Profile::LoadFile(map, path);
    return SummariseProfile(map);
  } catch (...) {
    return {};
  }
}

void
ProfileScreen::Step(int delta) noexcept
{
  const int index = profiles.Find(profiles.GetValue()) + delta;
  if (index < 0 || index >= int(profiles.size()))
    return;

  profiles.SetIndex(index);
  OnSelectionChanged();
}

void
ProfileScreen::OnSelectionChanged() noexcept
{
  summary = LoadSummary(profiles.GetValue());
  Invalidate();
}

void
ProfileScreen::OnCreate()
{
  ContainerWindow::OnCreate();

  LoadFonts();
  summary = LoadSummary(profiles.GetValue());

  const auto create = [this](Button &button, const char *caption,
                             SoftButtonRenderer::Badge badge,
                             std::function<void()> callback){
    button.Create(*this, {0, 0, 1, 1}, WindowStyle(),
                  std::make_unique<SoftButtonRenderer>(soft_key_font, caption,
                                                       badge, grid),
                  std::move(callback));
  };

  create(quit_button, _("Quit"), SoftButtonRenderer::Badge::CROSS, on_quit);
  create(select_button, _("Select"), SoftButtonRenderer::Badge::CHECK,
         on_select);
}

void
ProfileScreen::LoadFonts() noexcept
{
  const auto load = [this](Font &font, unsigned design_size){
    font.Load(FontDescription(grid.Scale(design_size)));
  };

  load(title_font, LX::TITLE_FONT);
  load(value_font, LX::VALUE_FONT);
  load(info_font, LX::INFO_FONT);
  load(soft_key_font, LX::SOFT_KEY_FONT);
}

void
ProfileScreen::OnResize(PixelSize new_size) noexcept
{
  ContainerWindow::OnResize(new_size);

  grid = DesignGrid(new_size, {LX::GRID_WIDTH, LX::GRID_HEIGHT});
  LoadFonts();

  using namespace LX::ProfilePage;
  field_rc = grid.Rect(FIELD_LEFT, FIELD_TOP, FIELD_RIGHT, FIELD_BOTTOM);

  LayoutSoftKeys(new_size);
}

/**
 * The button windows are larger than the visible soft keys, to be
 * touchable.
 */
void
ProfileScreen::LayoutSoftKeys(PixelSize size) noexcept
{
  const int width = grid.Scale(LX::SoftKey::SLOT_PITCH);
  const int height = std::max(grid.Scale(LX::SoftKey::HEIGHT +
                                         LX::Badge::OVERHANG),
                              int(Layout::GetMinimumControlHeight()));
  const int key_width = grid.Scale(LX::SoftKey::WIDTH);

  const auto place = [&](Button &button, unsigned slot_x){
    static_cast<SoftButtonRenderer &>(button.GetRenderer())
      .SetGrid(soft_key_font, grid);

    const int left = grid.X(slot_x) - (width - key_width) / 2;
    button.Move({left, int(size.height) - height, left + width,
                 int(size.height)});
  };

  place(quit_button, LX::SoftKey::FIRST_SLOT_X);
  place(select_button, LX::SoftKey::LAST_SLOT_X);
}

bool
ProfileScreen::OnMouseDown(PixelPoint p) noexcept
{
  if (!field_rc.Contains(p))
    return ContainerWindow::OnMouseDown(p);

  field_pressed = true;
  SetCapture();
  return true;
}

bool
ProfileScreen::OnMouseUp(PixelPoint p) noexcept
{
  if (!field_pressed)
    return ContainerWindow::OnMouseUp(p);

  /* like all XCSoar buttons: act on lift-off, inside the field only */
  field_pressed = false;
  ReleaseCapture();
  if (field_rc.Contains(p))
    on_field();
  return true;
}

void
ProfileScreen::OnCancelMode() noexcept
{
  field_pressed = false;
  ContainerWindow::OnCancelMode();
}

void
ProfileScreen::DrawText(Canvas &canvas, const Font &font, int x,
                        unsigned baseline, const char *text,
                        int max_width) const noexcept
{
  canvas.Select(font);
  canvas.DrawClippedText({x, grid.Y(baseline) - int(font.GetAscentHeight())},
                         unsigned(std::max(0, max_width)), text);
}

void
ProfileScreen::DrawField(Canvas &canvas) const noexcept
{
  using namespace LX::ProfilePage;

  DrawGradientBox(canvas, field_rc, LX::FIELD_GRADIENT_TOP,
                  LX::FIELD_GRADIENT_BOTTOM, LX::FIELD_BORDER,
                  grid.Scale(LX::FIELD_RADIUS));

  /* the profile name is always selected: a highlight as wide as the
     text, filling the inside of the field */
  const std::string name = GetProfileName(profiles.GetValue());
  const int x = grid.X(VALUE_X);
  const int width = std::min(int(value_font.TextSize(name.c_str()).width),
                             field_rc.right - 1 - x);
  canvas.DrawFilledRectangle({x, field_rc.top + 1, x + width,
                              field_rc.bottom - 1}, LX::SELECTION);
  DrawText(canvas, value_font, x, VALUE_BASELINE, name.c_str(), width);
}

void
ProfileScreen::DrawSummary(Canvas &canvas) const noexcept
{
  using namespace LX::ProfilePage;

  unsigned baseline = SUMMARY_BASELINE;
  for (const auto &line : summary) {
    if (!line.empty())
      DrawText(canvas, info_font, grid.X(SUMMARY_X), baseline, line.c_str(),
               field_rc.right - grid.X(SUMMARY_X));
    baseline += SUMMARY_PITCH;
  }
}

void
ProfileScreen::OnPaint(Canvas &canvas) noexcept
{
  using namespace LX::ProfilePage;

  canvas.Clear(LX::BACKGROUND);
  canvas.SetBackgroundTransparent();
  canvas.SetTextColor(LX::TEXT);

  const char *title = _("Profile");
  DrawText(canvas, title_font,
           grid.X(LX::GRID_WIDTH / 2) -
           int(title_font.TextSize(title).width) / 2,
           LX::TITLE_BASELINE, title);
  canvas.DrawFilledRectangle({0, grid.Y(LX::TITLE_RULE_Y),
                              int(canvas.GetWidth()),
                              grid.Y(LX::TITLE_RULE_Y) +
                              std::max(1, grid.Scale(1))}, LX::TEXT);

  DrawPeopleIcon(canvas, grid, ICON_X, ICON_Y, ICON);
  canvas.SetTextColor(LX::TEXT);

  DrawText(canvas, title_font, grid.X(LABEL_X), LABEL_BASELINE,
           (std::string{_("Select profile")} + ":").c_str());

  DrawField(canvas);
  DrawSummary(canvas);

  /* the soft keys */
  ContainerWindow::OnPaint(canvas);
}
