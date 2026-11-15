// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include <string>
#include <vector>

class ProfileMap;

/**
 * Summarise a profile for a preview, as lines of text: the glider,
 * the pilot with the safety values (in the profile's own units), and
 * the data files.  The paragraphs are separated by an empty line.
 * Missing data is left out.
 */
std::vector<std::string>
SummariseProfile(const ProfileMap &map) noexcept;
