// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include <memory>

class Widget;

/** Airspace warnings; the display settings are in AirspaceConfigPanel. */
std::unique_ptr<Widget>
CreateAirspaceWarningsConfigPanel();
