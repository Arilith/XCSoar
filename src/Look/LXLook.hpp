// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ui/canvas/Color.hpp"

/**
 * Colours and metrics of the LXNAV LX9000 user interface.  All
 * lengths are pixels of the 480x788 design grid (see #DesignGrid).
 */
namespace LX {

static constexpr unsigned GRID_WIDTH = 480;
static constexpr unsigned GRID_HEIGHT = 788;

static constexpr Color BACKGROUND = COLOR_BLACK;
static constexpr Color TEXT = COLOR_WHITE;
static constexpr Color SELECTION{0x31, 0x69, 0xc6};

/** Text sizes (em height) */
static constexpr unsigned TITLE_FONT = 26;
static constexpr unsigned LABEL_FONT = 26;
static constexpr unsigned VALUE_FONT = 38;
static constexpr unsigned INFO_FONT = 18;
static constexpr unsigned SOFT_KEY_FONT = 20;

/** Title bar: baseline of the title and the rule below it */
static constexpr unsigned TITLE_BASELINE = 27;
static constexpr unsigned TITLE_RULE_Y = 33;

/** A selection field with a white outline and a grey gradient */
static constexpr Color FIELD_BORDER = COLOR_WHITE;
static constexpr Color FIELD_GRADIENT_TOP{0xff, 0xff, 0xff};
static constexpr Color FIELD_GRADIENT_BOTTOM{0x81, 0x81, 0x81};
static constexpr unsigned FIELD_RADIUS = 3;

/** Soft keys on the bottom edge */
namespace SoftKey {

static constexpr Color TOP{0xfe, 0xfe, 0xfe};
static constexpr Color BOTTOM{0xe1, 0xe1, 0xe1};
static constexpr Color BORDER{0x6e, 0x6e, 0x6e};
static constexpr Color PRESSED = SELECTION;
static constexpr Color DISABLED_TEXT{0x80, 0x80, 0x80};
static constexpr unsigned RADIUS = 2;

static constexpr unsigned WIDTH = 100;
static constexpr unsigned HEIGHT = 26;

/** Distance from the top of the key to the text baseline */
static constexpr unsigned BASELINE = 21;

/** The keys form a grid of four slots */
static constexpr unsigned SLOT_PITCH = 112;
static constexpr unsigned FIRST_SLOT_X = 22;
static constexpr unsigned LAST_SLOT_X = FIRST_SLOT_X + 3 * SLOT_PITCH;

} // namespace SoftKey

/** The small coloured marker sitting on top of a soft key */
namespace Badge {

static constexpr unsigned SIZE = 14;
static constexpr unsigned OVERHANG = 7;
static constexpr unsigned RADIUS = 3;
static constexpr Color GLYPH = COLOR_WHITE;

struct Palette {
  Color fill, edge, lower_edge;
};

static constexpr Palette GREEN{{0x9a, 0xc2, 0x01}, {0x68, 0x83, 0x01},
                               {0xba, 0xd5, 0x56}};
static constexpr Palette RED{{0xd8, 0x43, 0x30}, {0x8e, 0x26, 0x1a},
                             {0xe8, 0x8c, 0x80}};

} // namespace Badge

/** The "Profile" page */
namespace ProfilePage {

static constexpr unsigned LABEL_X = 140;
static constexpr unsigned LABEL_BASELINE = 120;

static constexpr unsigned FIELD_LEFT = 144;
static constexpr unsigned FIELD_TOP = 132;
static constexpr unsigned FIELD_RIGHT = 444;
static constexpr unsigned FIELD_BOTTOM = 179;

/** Where the selection highlight and the value text start */
static constexpr unsigned VALUE_X = 147;
static constexpr unsigned VALUE_BASELINE = 171;

static constexpr unsigned SUMMARY_X = FIELD_LEFT;
static constexpr unsigned SUMMARY_BASELINE = 211;
static constexpr unsigned SUMMARY_PITCH = 22;

static constexpr unsigned ICON_X = 41;
static constexpr unsigned ICON_Y = 128;
static constexpr Color ICON{0xce, 0xe7, 0x72};

} // namespace ProfilePage

} // namespace LX
