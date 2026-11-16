// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "String.hpp"

#include <functional>
#include <string>
#include <vector>

class PrefixDataField final : public DataFieldString {
public:
  typedef std::function<const char *(const char *)> AllowedCharactersFunction;
  using SuggestionsFunction =
    std::function<void(const char *, std::vector<std::string> &)>;

private:
  AllowedCharactersFunction allowed_characters;
  SuggestionsFunction suggestions;

public:
  PrefixDataField(const char *value,
                  AllowedCharactersFunction _allowed_characters,
                  DataFieldListener *listener=nullptr) noexcept
    :DataFieldString(Type::PREFIX, value, listener),
     allowed_characters(_allowed_characters) {}

  PrefixDataField(const char *value="",
                  DataFieldListener *listener=nullptr) noexcept
    :DataFieldString(Type::PREFIX, value, listener) {}

  const AllowedCharactersFunction &GetAllowedCharactersFunction() const noexcept {
    return allowed_characters;
  }

  void SetSuggestionsFunction(SuggestionsFunction value) noexcept {
    suggestions = std::move(value);
  }

  const SuggestionsFunction &GetSuggestionsFunction() const noexcept {
    return suggestions;
  }

  /* virtual methods from class DataField */
  void Inc() noexcept override;
  void Dec() noexcept override;
  const char *GetAsDisplayString() const noexcept override;

protected:
  [[gnu::pure]]
  const char *GetAllowedCharacters() const noexcept {
    return allowed_characters
      ? allowed_characters("")
      : "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  }
};
