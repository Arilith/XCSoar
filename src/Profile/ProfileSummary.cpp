// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "ProfileSummary.hpp"
#include "Formatter/Units.hpp"
#include "Keys.hpp"
#include "Language/Language.hpp"
#include "Map.hpp"
#include "Plane/Plane.hpp"
#include "Plane/PlaneFileGlue.hpp"
#include "UnitsConfig.hpp"
#include "Units/Settings.hpp"
#include "system/Path.hpp"
#include "util/IterableSplitString.hxx"
#include "util/UTF8.hpp"

using Paragraph = std::vector<std::string>;

/**
 * Append "label: value" unless the value is empty or not valid UTF-8
 * (the profile is user-editable text).
 */
static void
AddLine(Paragraph &lines, const char *label, const char *value)
{
  if (value != nullptr && *value != '\0' && ValidateUTF8(value))
    lines.emplace_back(std::string{label} + ": " + value);
}

/**
 * The base names of a '|' separated list of files, joined by commas.
 */
static std::string
BaseNames(const char *list)
{
  std::string names;
  for (const auto item : TIterableSplitString(list, '|')) {
    if (item.empty())
      continue;

    if (!names.empty())
      names += ", ";

    const std::string file{item};
    names += Path(file.c_str()).GetBase().c_str();
  }

  return names;
}

static void
AddFileLine(Paragraph &lines, const char *label, const ProfileMap &map,
            std::string_view key)
{
  if (const char *list = map.Get(key))
    AddLine(lines, label, BaseNames(list).c_str());
}

/**
 * Read the glider from the plane file; older profiles keep it in the
 * profile itself.
 */
static Plane
LoadPlane(const ProfileMap &map) noexcept
{
  Plane plane{};

  if (const auto path = map.GetPath("PlanePath");
      path != nullptr && PlaneGlue::ReadFile(plane, path))
    return plane;

  plane = {};
  plane.type.SetUTF8(map.Get(ProfileKeys::AircraftType, ""));
  plane.competition_id.SetUTF8(map.Get(ProfileKeys::CompetitionId, ""));
  plane.registration.SetUTF8(map.Get(ProfileKeys::AircraftReg, ""));
  return plane;
}

static Paragraph
GliderParagraph(const ProfileMap &map)
{
  const Plane plane = LoadPlane(map);

  Paragraph lines;
  AddLine(lines, _("Glider"), plane.type);
  AddLine(lines, _("Competition ID"), plane.competition_id);
  AddLine(lines, _("Registration"), plane.registration);
  return lines;
}

static Paragraph
PilotParagraph(const ProfileMap &map)
{
  UnitSetting units;
  Profile::LoadUnits(map, units);

  Paragraph lines;
  AddLine(lines, _("Pilot"), map.Get(ProfileKeys::PilotName));

  char buffer[32];
  if (double height; map.Get(ProfileKeys::SafetyAltitudeArrival, height)) {
    FormatAltitude(buffer, height, units.altitude_unit);
    AddLine(lines, _("Arrival height"), buffer);
  }

  /* stored in tenths of a metre per second */
  if (unsigned mc; map.Get(ProfileKeys::SafetyMacCready, mc)) {
    FormatVerticalSpeed(buffer, mc / 10., units.vertical_speed_unit);
    AddLine(lines, _("Safety MC"), buffer);
  }

  return lines;
}

static Paragraph
FilesParagraph(const ProfileMap &map)
{
  Paragraph lines;
  AddFileLine(lines, _("Waypoints"), map, ProfileKeys::WaypointFileList);
  AddFileLine(lines, _("Airspace"), map, ProfileKeys::AirspaceFileList);
  AddFileLine(lines, _("Map"), map, ProfileKeys::MapFile);
  return lines;
}

std::vector<std::string>
SummariseProfile(const ProfileMap &map) noexcept
{
  std::vector<std::string> lines;

  try {
    for (const auto &paragraph : {GliderParagraph(map), PilotParagraph(map),
                                  FilesParagraph(map)}) {
      if (paragraph.empty())
        continue;

      if (!lines.empty())
        lines.emplace_back();

      lines.insert(lines.end(), paragraph.begin(), paragraph.end());
    }
  } catch (...) {
    /* out of memory: show no summary */
    lines.clear();
  }

  return lines;
}
