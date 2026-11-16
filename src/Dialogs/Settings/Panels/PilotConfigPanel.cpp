// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "PilotConfigPanel.hpp"
#include "Profile/Profile.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"

enum ControlIndex {
  PilotName,
  CoPilotName,
};

class PilotConfigPanel final : public RowFormWidget {
public:
  PilotConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  void Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept override;
  bool Save(bool &changed) noexcept override;
};

void
PilotConfigPanel::Prepare(ContainerWindow &parent,
                          const PixelRect &rc) noexcept
{
  const LoggerSettings &logger =
    CommonInterface::GetComputerSettings().logger;

  RowFormWidget::Prepare(parent, rc);
  AddText(_("Pilot name"),
          _("Name of the pilot in command, recorded in the IGC flight log."),
          logger.pilot_name);

  AddText(_("CoPilot name"),
          _("The co-pilot name recorded in the IGC flight log."),
          logger.copilot_name);
}

bool
PilotConfigPanel::Save(bool &changed) noexcept
{
  LoggerSettings &logger =
    CommonInterface::SetComputerSettings().logger;

  changed |= SaveValue(PilotName, ProfileKeys::PilotName,
                       logger.pilot_name);

  changed |= SaveValue(CoPilotName, ProfileKeys::CoPilotName,
                       logger.copilot_name);

  return true;
}

std::unique_ptr<Widget>
CreatePilotConfigPanel()
{
  return std::make_unique<PilotConfigPanel>();
}
