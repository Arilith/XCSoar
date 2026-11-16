// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Look.hpp"
#include "UISettings.hpp"
#include "GlobalSettings.hpp"

void
Look::Initialise(const Font &map_font)
{
  dialog.Initialise();
  traffic.Initialise(map_font);
  gesture.Initialise();
  chart.Initialise();
}

[[gnu::pure]]
static bool
GetDarkMode(const UISettings &settings) noexcept
{
#ifdef KOBO
  (void)settings;
  return false;
#else
  switch (settings.dark_mode) {
  case UISettings::DarkMode::OFF:
  case UISettings::DarkMode::COUNT:
    break;

  case UISettings::DarkMode::ON:
    return true;

  case UISettings::DarkMode::AUTO:
    return GlobalSettings::dark_mode;
  }

  return false;
#endif
}

[[gnu::pure]]
static bool
GetInfoBoxDarkMode(const UISettings &settings, bool dark_mode) noexcept
{
  switch (settings.info_boxes.theme) {
  case InfoBoxSettings::Theme::FOLLOW_GLOBAL:
    return dark_mode;

  case InfoBoxSettings::Theme::LIGHT:
    return false;

  case InfoBoxSettings::Theme::DARK:
    return true;
  }

  return dark_mode;
}

/**
 * Are the InfoBoxes drawn as the LX navbox strip over the map?  That
 * needs the LX look and OpenGL, whose alpha blending lets the map
 * shine through.
 */
[[gnu::const]]
static bool
IsNavBoxStrip([[maybe_unused]] bool strip_geometry,
              [[maybe_unused]] bool lx,
              [[maybe_unused]] bool infobox_dark_mode) noexcept
{
#ifdef ENABLE_OPENGL
  return strip_geometry && lx && infobox_dark_mode;
#else
  return false;
#endif
}

void
Look::InitialiseConfigured(const UISettings &settings,
                           const Font &map_font, const Font &map_bold_font,
                           unsigned infobox_width, bool navbox_strip)
{
  const bool dark_mode = GetDarkMode(settings);
  const bool infobox_dark_mode = GetInfoBoxDarkMode(settings, dark_mode);

  dialog.Initialise(dark_mode);
  chart.Initialise(dark_mode);
  terminal.Initialise();
  cross_section.Initialise(map_font);
  horizon.Initialise();
  thermal_band.Initialise(dark_mode,
                          cross_section.sky_color);
  trace_history.Initialise(dark_mode);
  info_box.Initialise(infobox_dark_mode, settings.info_boxes.use_colors,
                      infobox_width,
                      settings.info_boxes.scale_title_font,
                      IsNavBoxStrip(navbox_strip, dialog.lx,
                                    infobox_dark_mode));
  vario.Initialise(infobox_dark_mode,
                   settings.info_boxes.use_colors,
                   infobox_width,
                   info_box.title_font);
  wind_arrow_info_box.Initialise(map_bold_font, infobox_dark_mode);
  next_arrow_info_box.Initialise(map_bold_font,
                                 settings.info_boxes.use_colors,
                                 infobox_dark_mode);
  flarm_gauge.Initialise(traffic, true, dark_mode);
  flarm_dialog.Initialise(traffic, false, dark_mode);
  thermal_assistant_dialog.Initialise(false, dark_mode);
  thermal_assistant_gauge.Initialise(true, dark_mode);
  final_glide_bar.Initialise(map_bold_font);
  vario_bar.Initialise(map_bold_font);
  map.Initialise(settings.map, map_font, map_bold_font, dialog.lx);
  icon.Initialise();
  circling_percent.Initialise();
}

void
Look::ReinitialiseLayout(unsigned infobox_width, unsigned scale_title_font,
                         bool navbox_strip)
{
  /* Fonts and pens follow Layout::.  Reload after a resize, DPI
     change, or Text size change. */
  dialog.LoadFonts();
  horizon.Initialise();

  info_box.ReinitialiseLayout(infobox_width, scale_title_font,
                              IsNavBoxStrip(navbox_strip, dialog.lx,
                                            info_box.inverse));
  vario.ReinitialiseLayout(infobox_width);
  flarm_gauge.ReinitialiseLayout();
  flarm_dialog.ReinitialiseLayout();
}
