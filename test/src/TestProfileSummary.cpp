// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Profile/ProfileSummary.hpp"
#include "Profile/Keys.hpp"
#include "Profile/Map.hpp"
#include "Units/Unit.hpp"
#include "TestUtil.hpp"

static bool
Equals(const std::vector<std::string> &lines,
       const std::vector<std::string> &expected)
{
  return lines == expected;
}

static void
TestEmpty()
{
  ProfileMap map;
  ok1(SummariseProfile(map).empty());
}

static void
TestParagraphs()
{
  ProfileMap map;
  map.Set(ProfileKeys::AircraftType, "ASK 21");
  map.Set(ProfileKeys::CompetitionId, "B21");
  map.Set(ProfileKeys::PilotName, "Bob");
  /* do not depend on the units of the OS language */
  map.Set(ProfileKeys::LiftUnitsValue, int(Unit::METER_PER_SECOND));
  map.Set(ProfileKeys::SafetyMacCready, "5");
  map.Set(ProfileKeys::WaypointFileList, "a/first.cup|second.cup");
  map.Set(ProfileKeys::MapFile, "alps.xcm");

  ok1(Equals(SummariseProfile(map),
             {"Glider: ASK 21", "Competition ID: B21", "",
              "Pilot: Bob", "Safety MC: +0.5 m/s", "",
              "Waypoints: first.cup, second.cup", "Map: alps.xcm"}));
}

static void
TestOmitsAbsentParagraph()
{
  ProfileMap map;
  map.Set(ProfileKeys::PilotName, "Anna");
  map.Set(ProfileKeys::AirspaceFileList, "europe.txt");

  ok1(Equals(SummariseProfile(map),
             {"Pilot: Anna", "", "Airspace: europe.txt"}));
}

static void
TestInvalidUTF8()
{
  ProfileMap map;
  map.Set(ProfileKeys::PilotName, "\xff\xfe");
  ok1(SummariseProfile(map).empty());
}

int
main()
{
  plan_tests(4);

  TestEmpty();
  TestParagraphs();
  TestOmitsAbsentParagraph();
  TestInvalidUTF8();

  return exit_status();
}
