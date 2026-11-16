// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "AirspaceConfigPanel.hpp"
#include "ConfigPanel.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Form/DataField/Listener.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Dialogs/Airspace/Airspace.hpp"
#include "Profile/Keys.hpp"
#include "Language/Language.hpp"
#include "Airspace/AirspaceComputerSettings.hpp"
#include "Renderer/AirspaceRendererSettings.hpp"
#include "Interface.hpp"
#include "UIGlobals.hpp"

using namespace std::chrono;

enum ControlIndex {
  AirspaceDisplay,
  ClipAltitude,
  AltWarningMargin,
  AirspaceLabelSelection,
#ifdef HAVE_HTTP
  ShowNotamLabels,
#endif
  SPACER_APPEARANCE,
  UseBlackOutline,
  AirspaceFillMode,
};

static constexpr StaticEnumChoice as_display_list[] = {
  { AirspaceDisplayMode::ALLON, N_("All on"),
    N_("All airspaces are displayed.") },
  { AirspaceDisplayMode::CLIP, N_("Clip"),
    N_("Display airspaces below the clip altitude.") },
  { AirspaceDisplayMode::AUTO, NC_("Setting", "Auto"),
    N_("Display airspaces within a margin of the glider.") },
  { AirspaceDisplayMode::ALLBELOW, N_("All below"),
    N_("Display airspaces below the glider or within a margin.") },
  nullptr
};

static constexpr StaticEnumChoice as_fill_mode_list[] = {
  { AirspaceRendererSettings::FillMode::DEFAULT, N_("Default"),
    N_("This selects the best performing option for your hardware. "
      "In fact it favours 'fill padding' except for PPC 2000 system.") },
  { AirspaceRendererSettings::FillMode::ALL, N_("Fill all"),
    N_("Transparently fills the airspace colour over the whole area.") },
  { AirspaceRendererSettings::FillMode::PADDING, N_("Fill padding"),
    N_("Draws a solid outline with a half transparent border around the airspace.") },
  { AirspaceRendererSettings::FillMode::NONE, N_("No fill"),
    N_("Don't fill the airspace area.") },
  nullptr
};

static constexpr StaticEnumChoice as_label_selection_list[] = {
  { AirspaceRendererSettings::LabelSelection::NONE, N_("None"),
    N_("No labels will be displayed.") },
  { AirspaceRendererSettings::LabelSelection::ALL, N_("All"),
    N_("All labels will be displayed.") },
  nullptr
};

class AirspaceConfigPanel final
  : public RowFormWidget, DataFieldListener {
public:
  AirspaceConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

  void ShowDisplayControls(AirspaceDisplayMode mode);

  /* methods from Widget */
  void Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept override;
  bool Save(bool &changed) noexcept override;
  void Show(const PixelRect &rc) noexcept override;
  void Hide() noexcept override;

private:
  /* methods from DataFieldListener */
  void OnModified(DataField &df) noexcept override;
};

void
AirspaceConfigPanel::ShowDisplayControls(AirspaceDisplayMode mode)
{
  SetRowAvailable(ClipAltitude,
                  mode == AirspaceDisplayMode::CLIP);

  SetRowAvailable(AltWarningMargin,
                  mode == AirspaceDisplayMode::AUTO ||
                  mode == AirspaceDisplayMode::ALLBELOW);
}

void
AirspaceConfigPanel::Show(const PixelRect &rc) noexcept
{
  ConfigPanel::BorrowExtraButton(1, _("Colours"), [](){
    dlgAirspaceShowModal(true);
  });

  ConfigPanel::BorrowExtraButton(2, _("Filter"), [](){
    dlgAirspaceShowModal(false);
  });

  RowFormWidget::Show(rc);
}

void
AirspaceConfigPanel::Hide() noexcept
{
  RowFormWidget::Hide();
  ConfigPanel::ReturnExtraButton(1);
  ConfigPanel::ReturnExtraButton(2);
}

void
AirspaceConfigPanel::OnModified(DataField &df) noexcept
{
  if (IsDataField(AirspaceDisplay, df)) {
    const DataFieldEnum &dfe = (const DataFieldEnum &)df;
    AirspaceDisplayMode mode = (AirspaceDisplayMode)dfe.GetValue();
    ShowDisplayControls(mode);
  }
}

void
AirspaceConfigPanel::Prepare(ContainerWindow &parent,
                             const PixelRect &rc) noexcept
{
  const AirspaceComputerSettings &computer =
    CommonInterface::GetComputerSettings().airspace;
  const AirspaceRendererSettings &renderer =
    CommonInterface::GetMapSettings().airspace;

  RowFormWidget::Prepare(parent, rc);

  AddEnum(_("Airspace display"),
          _("Controls filtering of airspace for display and warnings. The airspace filter button also allows filtering of display and warnings independently for each airspace class."),
          as_display_list, (unsigned)renderer.altitude_mode, this);

  AddFloat(_("Clip altitude"),
           _("For clip airspace mode, this is the altitude below which airspace is displayed."),
           "%.0f %s", "%.0f", 0, 20000, 100, false,
           UnitGroup::ALTITUDE, renderer.clip_altitude);

  AddFloat(_("Margin"),
           _("For auto and all below airspace mode, this is the altitude above/below which airspace is included."),
           "%.0f %s", "%.0f", 0, 10000, 100, false,
           UnitGroup::ALTITUDE, computer.warnings.altitude_warning_margin);

  AddEnum(_("Label visibility"),
          _("Determines what labels are displayed."),
          as_label_selection_list, (unsigned)renderer.label_selection);
  SetExpertRow(AirspaceLabelSelection);

#ifdef HAVE_HTTP
  AddBoolean(_("Show NOTAM labels"),
             _("Show brief NOTAM text labels on the map when zoomed in sufficiently."),
             renderer.show_notam_labels);
  SetExpertRow(ShowNotamLabels);
#endif

  AddSpacer();
  SetExpertRow(SPACER_APPEARANCE);

  AddBoolean(_("Use black outline"),
             _("Draw a black outline around each airspace rather than the airspace color."),
             renderer.black_outline);
  SetExpertRow(UseBlackOutline);

  AddEnum(_("Airspace fill mode"),
          _("Specifies the mode for filling the airspace area."),
          as_fill_mode_list, (unsigned)renderer.fill_mode);
  SetExpertRow(AirspaceFillMode);

  ShowDisplayControls(renderer.altitude_mode);
}


bool
AirspaceConfigPanel::Save(bool &_changed) noexcept
{
  bool changed = false;

  AirspaceComputerSettings &computer =
    CommonInterface::SetComputerSettings().airspace;
  AirspaceRendererSettings &renderer =
    CommonInterface::SetMapSettings().airspace;

  changed |= SaveValueEnum(AirspaceDisplay, ProfileKeys::AltMode, renderer.altitude_mode);

  changed |= SaveValueEnum(AirspaceLabelSelection, ProfileKeys::AirspaceLabelSelection, renderer.label_selection);

#ifdef HAVE_HTTP
  changed |= SaveValue(ShowNotamLabels, ProfileKeys::AirspaceShowNOTAMLabels,
                       renderer.show_notam_labels);
#endif

  changed |= SaveValue(ClipAltitude, UnitGroup::ALTITUDE, ProfileKeys::ClipAlt, renderer.clip_altitude);

  changed |= SaveValue(AltWarningMargin, UnitGroup::ALTITUDE, ProfileKeys::AltMargin, computer.warnings.altitude_warning_margin);

  changed |= SaveValue(UseBlackOutline, ProfileKeys::AirspaceBlackOutline, renderer.black_outline);

  changed |= SaveValueEnum(AirspaceFillMode, ProfileKeys::AirspaceFillMode, renderer.fill_mode);

  _changed |= changed;

  return true;
}

std::unique_ptr<Widget>
CreateAirspaceConfigPanel()
{
  return std::make_unique<AirspaceConfigPanel>();
}
