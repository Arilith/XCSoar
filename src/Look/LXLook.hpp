// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ui/canvas/Color.hpp"
#include "Screen/Layout.hpp"

#include <cstdint>

/**
 * Colours and metrics of the LXNAV LX9000 user interface.  All
 * lengths are pixels of the 480x788 design grid (see #DesignGrid).
 */
namespace LX {

static constexpr unsigned GRID_WIDTH = 480;
static constexpr unsigned GRID_HEIGHT = 788;

/**
 * Convert a length of the design grid to pixels of the current
 * screen.  Unlike #DesignGrid this does not need the size of a
 * window: the short edge of the screen plays the role of the grid
 * width, so a form looks the same in portrait and landscape.
 */
[[gnu::pure]]
static inline unsigned
Scale(unsigned design) noexcept
{
  return (design * Layout::min_screen_pixels + GRID_WIDTH / 2) / GRID_WIDTH;
}

/**
 * A font size of the design grid in pixels, additionally scaled by
 * the text size setting of the user.
 */
[[gnu::pure]]
static inline unsigned
FontSize(unsigned design) noexcept
{
  return Scale(design) * Layout::ui_scale / 100;
}

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
static constexpr Color TITLE_RULE = COLOR_WHITE;

/**
 * The text of dialogs: field values, list rows and body text.  LX
 * shows field values at 32 and list rows at 28; the longer strings of
 * XCSoar need the smaller size to fit into half a screen.
 */
static constexpr unsigned DIALOG_FONT = 28;

/** A selection field with a white outline and a grey gradient */
static constexpr Color FIELD_BORDER = COLOR_WHITE;
static constexpr Color FIELD_GRADIENT_TOP{0xff, 0xff, 0xff};
static constexpr Color FIELD_GRADIENT_BOTTOM{0x81, 0x81, 0x81};
static constexpr unsigned FIELD_RADIUS = 3;

/** A value field of a settings form */
namespace Field {

static constexpr Color TOP{0x20, 0x20, 0x20};
static constexpr Color BOTTOM{0x11, 0x11, 0x11};

/** The outline of the dark field, between #TOP and #BOTTOM */
static constexpr Color EDGE{0x18, 0x18, 0x18};

/** Fill of a value that cannot be edited */
static constexpr Color FLAT{0x20, 0x20, 0x20};

/** The field while its value is being edited */
static constexpr Color ACTIVE_TOP{0xff, 0xff, 0xff};
static constexpr Color ACTIVE_BOTTOM{0x84, 0x84, 0x84};

static constexpr Color TEXT = COLOR_WHITE;
static constexpr Color DISABLED_TEXT{0x80, 0x80, 0x80};
static constexpr Color FOCUS_RING = COLOR_WHITE;

static constexpr unsigned HEIGHT = 36;
static constexpr unsigned RADIUS = 3;

/** The focus ring is drawn this far outside the field */
static constexpr unsigned RING_OUTSET = 1;

/** Distance between the field edge and the value text */
static constexpr unsigned TEXT_PADDING = 4;

/** Distance between the field edge and the highlight of the value */
static constexpr unsigned HIGHLIGHT_INSET = 2;

} // namespace Field

/** The layout of a settings form */
namespace Form {

static constexpr unsigned MARGIN = 6;
static constexpr unsigned COLUMN_GAP = 8;
static constexpr unsigned ROW_PITCH = 45;

/** A label is right-aligned this far left of its field */
static constexpr unsigned LABEL_GAP = 6;

/** Width of the field column when labels stand left of the fields */
static constexpr unsigned VALUE_COLUMN_WIDTH = 172;

/**
 * Below this width a column is too narrow to read; the form stays
 * in one column.
 */
static constexpr unsigned MIN_CELL_WIDTH = 200;

} // namespace Form

/** The check box glyph */
namespace CheckBox {

static constexpr unsigned BOX_SIZE = 19;
static constexpr unsigned OUTLINE = 2;

/** Distance between the left edge of the row and the glyph */
static constexpr unsigned PADDING = 5;

/** Distance between the glyph and the caption */
static constexpr unsigned CAPTION_GAP = 12;

} // namespace CheckBox

/** Soft keys on the bottom edge */
namespace SoftKey {

static constexpr Color TOP{0xfe, 0xfe, 0xfe};
static constexpr Color BOTTOM{0xe1, 0xe1, 0xe1};
static constexpr Color BORDER{0x6e, 0x6e, 0x6e};
static constexpr Color PRESSED = SELECTION;
static constexpr Color DISABLED_TEXT{0x80, 0x80, 0x80};
static constexpr unsigned RADIUS = 2;

/** The white ring marking the key with the keyboard focus */
static constexpr Color FOCUS_RING = COLOR_WHITE;
static constexpr unsigned RING_OUTSET = 1;

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

/** Backdrop of the translucent map InfoBox strip. */
namespace NavBox {
static constexpr uint8_t BACKDROP_ALPHA = 180;
}

/**
 * The on-screen keyboard: a panel of 10 x 5 cells, each cell holding
 * a key face smaller than the cell, so the touch target is larger
 * than what is drawn.
 */
namespace Keyboard {

static constexpr Color PANEL{0x10, 0x10, 0x10};

static constexpr unsigned PANEL_WIDTH = 472;
static constexpr unsigned PANEL_HEIGHT = 290;

/** Distance between the panel and the row of soft keys below it */
static constexpr unsigned PANEL_BOTTOM_GAP = 2;

static constexpr unsigned COLUMNS = 10;
static constexpr unsigned ROWS = 5;

/** The visible face of a key, and the pitch of the cells */
static constexpr unsigned KEY_WIDTH = 40;
static constexpr unsigned KEY_HEIGHT = 50;
static constexpr unsigned COLUMN_PITCH = 46;
static constexpr unsigned ROW_PITCH = 56;

static constexpr Color KEY_FILL{0x28, 0x28, 0x28};
static constexpr Color KEY_EDGE{0x30, 0x30, 0x30};
static constexpr Color ACTION_FILL{0x38, 0x38, 0x38};
static constexpr Color ACTION_EDGE{0x40, 0x40, 0x40};

/** The dark line around a face; white on the key with the focus */
static constexpr Color KEY_OUTLINE{0x01, 0x01, 0x01};
static constexpr Color FOCUS_RING = COLOR_WHITE;
static constexpr unsigned RADIUS = 3;

static constexpr Color TEXT{0xe8, 0xe8, 0xe8};
static constexpr Color DISABLED_TEXT{0x60, 0x60, 0x60};

/** Text size of the characters, and of the words on the action keys */
static constexpr unsigned FONT = 32;
static constexpr unsigned LABEL_FONT = 16;

/** The "space" glyph: a bracket open to the top, in the disabled grey */
static constexpr unsigned SPACE_GLYPH_WIDTH = 22;
static constexpr unsigned SPACE_GLYPH_HEIGHT = 6;
static constexpr unsigned SPACE_GLYPH_STROKE = 3;
static constexpr unsigned SPACE_GLYPH_TICK = 4;

/** Distance between the bottom of the face and the space glyph */
static constexpr unsigned SPACE_GLYPH_BOTTOM_INSET = 10;

/** The backspace glyph: an arrow pointing left with a cross in it */
static constexpr unsigned BACKSPACE_GLYPH_WIDTH = 27;
static constexpr unsigned BACKSPACE_GLYPH_HEIGHT = 18;

/** The field showing the text above the keyboard */
static constexpr unsigned EDITOR_TOP = 8;
static constexpr unsigned EDITOR_HEIGHT = 45;

} // namespace Keyboard

/** Dialogs which do not fill the screen */
namespace Dialog {

/** The top edge of a dialog */
static constexpr Color EDGE{0xe8, 0xe8, 0xe8};

/** The vertical gradient of the message box panel */
static constexpr Color MESSAGE_TOP{0x2c, 0x2c, 0x2c};
static constexpr Color MESSAGE_BOTTOM{0x13, 0x13, 0x13};

} // namespace Dialog

/** The rows of a list, on a flat panel */
namespace ListRow {

static constexpr Color PANEL{0x20, 0x20, 0x20};

/** The cursor row */
static constexpr Color TOP{0x3f, 0x3f, 0x3f};
static constexpr Color BOTTOM{0x22, 0x22, 0x22};
static constexpr Color BORDER = COLOR_WHITE;
static constexpr unsigned RADIUS = 3;

} // namespace ListRow

/** The scroll bar of a list */
namespace ScrollBar {

static constexpr unsigned WIDTH = 15;
static constexpr unsigned THUMB_WIDTH = 9;
static constexpr unsigned RADIUS = 3;
static constexpr Color BACKGROUND = COLOR_BLACK;
static constexpr Color OUTLINE = COLOR_WHITE;
static constexpr Color THUMB = COLOR_WHITE;

} // namespace ScrollBar

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

namespace LX {

/** The tiles of the "Setup" menu */
namespace Tile {

static constexpr Color TOP{0x22, 0x22, 0x22};
static constexpr Color BOTTOM{0x12, 0x12, 0x12};
static constexpr Color BORDER{0x44, 0x44, 0x44};

static constexpr Color FOCUS_TOP{0x43, 0x43, 0x43};
static constexpr Color FOCUS_BOTTOM{0x23, 0x23, 0x23};
static constexpr Color FOCUS_BORDER = COLOR_WHITE;

static constexpr unsigned WIDTH = 233;
static constexpr unsigned HEIGHT = 78;
static constexpr unsigned RADIUS = 2;

/** Distance between tiles, and between the grid and the left edge */
static constexpr unsigned GAP = 2;
static constexpr unsigned MARGIN = 1;

static constexpr unsigned LABEL_FONT = 18;

/** Baseline of the label, from the top of the tile */
static constexpr unsigned LABEL_BASELINE = 69;

/** The icon sits above the label */
static constexpr unsigned ICON_CENTER_Y = 28;
static constexpr unsigned ICON_HEIGHT = 36;

/** The scroll bar thumb right of the tiles */
static constexpr Color THUMB{0xc1, 0xc1, 0xc1};
static constexpr unsigned THUMB_WIDTH = 8;
static constexpr unsigned THUMB_MARGIN = 1;
static constexpr unsigned GUTTER = 11;

} // namespace Tile

/** A group box in a form: an outline with the caption on its top edge */
namespace Section {

static constexpr unsigned RADIUS = 6;

/** Distance between the outline and the form inside */
static constexpr unsigned PADDING = 4;

/** Empty space below the box */
static constexpr unsigned GAP = 8;

/** Distance between the left edge and the caption */
static constexpr unsigned CAPTION_X = 10;

} // namespace Section

} // namespace LX
