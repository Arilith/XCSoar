// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "ButtonRenderer.hpp"
#include "util/StaticString.hxx"

class Color;
class Font;

/**
 * A key of the on-screen keyboard in the style of the LXNAV LX9000
 * (see #LX::Keyboard): a dark face with a light label.  The face is
 * centred in the button rectangle, which is a whole cell of the
 * keyboard grid and therefore larger than the face.  A key which is
 * disabled loses its face and keeps a dim label; the focus is a white
 * ring around the face.
 */
class LXKeyRenderer final : public ButtonRenderer {
public:
  /** What a key shows. */
  enum class Glyph {
    /** the caption */
    TEXT,

    /** the "space" bracket */
    SPACE,

    /** a left arrow with a cross */
    BACKSPACE,
  };

  /** Characters and actions have different faces. */
  enum class Kind {
    CHARACTER,
    ACTION,
  };

private:
  const Font &font;
  StaticString<16> caption;
  Kind kind;
  Glyph glyph;

public:
  LXKeyRenderer(const Font &_font, const char *_caption, Kind _kind,
                Glyph _glyph=Glyph::TEXT) noexcept
    :font(_font), caption(_caption), kind(_kind), glyph(_glyph) {}

  /* virtual methods from class ButtonRenderer */
  void SetCaption(const char *_caption) noexcept override {
    caption = _caption;
  }

  void DrawButton(Canvas &canvas, const PixelRect &rc,
                  ButtonState state) const noexcept override;

private:
  void DrawCaption(Canvas &canvas, const PixelRect &face,
                   Color color) const noexcept;
  void DrawSpace(Canvas &canvas, const PixelRect &face,
                 Color color) const noexcept;
  void DrawBackspace(Canvas &canvas, const PixelRect &face,
                     Color color) const noexcept;
};
