// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include <memory>

class Widget;

/** The names of the pilot and the co-pilot. */
std::unique_ptr<Widget>
CreatePilotConfigPanel();
