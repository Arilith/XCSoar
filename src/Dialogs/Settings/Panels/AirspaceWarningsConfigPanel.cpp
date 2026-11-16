// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "AirspaceWarningsConfigPanel.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Form/DataField/Listener.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Profile/Keys.hpp"
#include "Language/Language.hpp"
#include "Airspace/AirspaceComputerSettings.hpp"
#include "Interface.hpp"
#include "UIGlobals.hpp"

using namespace std::chrono;

enum ControlIndex {
  AirspaceWarnings,
  WarningDialog,
  WarningTime,
  RepetitiveSound,
  AcknowledgeTime,
};

class AirspaceWarningsConfigPanel final
  : public RowFormWidget, DataFieldListener {
public:
  AirspaceWarningsConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

  void ShowWarningControls(bool visible);

  /* methods from Widget */
  void Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept override;
  bool Save(bool &changed) noexcept override;

private:
  /* methods from DataFieldListener */
  void OnModified(DataField &df) noexcept override;
};

void
AirspaceWarningsConfigPanel::ShowWarningControls(bool visible)
{
  SetRowAvailable(WarningDialog, visible);
  SetRowAvailable(WarningTime, visible);
  SetRowAvailable(RepetitiveSound, visible);
  SetRowAvailable(AcknowledgeTime, visible);
}

void
AirspaceWarningsConfigPanel::OnModified(DataField &df) noexcept
{
  if (IsDataField(AirspaceWarnings, df)) {
    const DataFieldBoolean &dfb = (const DataFieldBoolean &)df;
    ShowWarningControls(dfb.GetValue());
  }
}

void
AirspaceWarningsConfigPanel::Prepare(ContainerWindow &parent,
                                     const PixelRect &rc) noexcept
{
  const AirspaceComputerSettings &computer =
    CommonInterface::GetComputerSettings().airspace;
  const UISettings &ui_settings =
    CommonInterface::GetUISettings();

  RowFormWidget::Prepare(parent, rc);

  AddBoolean(_("Warnings"), _("Enable/disable all airspace warnings."),
             computer.enable_warnings, this);

  AddBoolean(_("Warnings dialog"),
             _("Enable/disable displaying airspaces warnings dialog."),
             ui_settings.enable_airspace_warning_dialog, this);
  SetExpertRow(WarningDialog);

  AddDuration(_("Warning time"),
              _("This is the time before an airspace incursion is estimated at which the system will warn the pilot."),
              seconds{10}, seconds{1000}, seconds{5},
              computer.warnings.warning_time);
  SetExpertRow(WarningTime);

  AddBoolean(_("Repetitive sound"),
             _("Enable/disable repetitive warning sound when airspaces warnings dialog is displayed."),
             computer.warnings.repetitive_sound, this);
  SetExpertRow(RepetitiveSound);

  AddDuration(_("Acknowledge time"),
              _("This is the time period in which an acknowledged airspace warning will not be repeated."),
              seconds{10}, seconds{1000}, seconds{5},
              computer.warnings.acknowledgement_time);
  SetExpertRow(AcknowledgeTime);

  ShowWarningControls(computer.enable_warnings);
}

bool
AirspaceWarningsConfigPanel::Save(bool &_changed) noexcept
{
  bool changed = false;

  AirspaceComputerSettings &computer =
    CommonInterface::SetComputerSettings().airspace;
  UISettings &ui_settings = CommonInterface::SetUISettings();

  changed |= SaveValue(AirspaceWarnings, ProfileKeys::AirspaceWarning,
                       computer.enable_warnings);

  changed |= SaveValue(WarningDialog, ProfileKeys::AirspaceWarningDialog,
                       ui_settings.enable_airspace_warning_dialog);

  changed |= SaveValue(WarningTime, ProfileKeys::WarningTime,
                       computer.warnings.warning_time);

  changed |= SaveValue(RepetitiveSound, ProfileKeys::RepetitiveSound,
                       computer.warnings.repetitive_sound);

  changed |= SaveValue(AcknowledgeTime, ProfileKeys::AcknowledgementTime,
                       computer.warnings.acknowledgement_time);

  _changed |= changed;

  return true;
}

std::unique_ptr<Widget>
CreateAirspaceWarningsConfigPanel()
{
  return std::make_unique<AirspaceWarningsConfigPanel>();
}
