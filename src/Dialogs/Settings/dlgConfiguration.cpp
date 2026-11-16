// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "Dialogs/Dialogs.h"
#include "Widget/ArrowPagerWidget.hpp"
#include "Widget/CreateWindowWidget.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "Look/DialogLook.hpp"
#include "Dialogs/FileManager.hpp"
#include "Dialogs/ProfileListDialog.hpp"
#include "Dialogs/ProfilePasswordDialog.hpp"
#include "Dialogs/Plane/PlaneDialogs.hpp"
#include "Dialogs/Device/DeviceListDialog.hpp"
#include "Dialogs/DataManagement/ExportFlightsPanel.hpp"
#include "Dialogs/DataManagement/ImportDataPanel.hpp"
#include "Dialogs/DataManagement/BackupRestorePanel.hpp"
#include "Dialogs/DataManagement/AdvancedFileExplorer.hpp"
#include "BackendComponents.hpp"
#include "Components.hpp"
#include "UIGlobals.hpp"
#include "ui/event/KeyCode.hpp"
#include "Form/TabMenuDisplay.hpp"
#include "Form/TabMenuData.hpp"
#include "Form/CheckBox.hpp"
#include "Form/Button.hpp"
#include "Renderer/TextButtonRenderer.hpp"
#include "Screen/Layout.hpp"
#include "Profile/Profile.hpp"
#include "util/Macros.hpp"
#include "util/ScopeExit.hxx"
#include "Panels/ConfigPanel.hpp"
#include "Panels/PagesConfigPanel.hpp"
#include "Panels/UnitsConfigPanel.hpp"
#include "Panels/TimeConfigPanel.hpp"
#include "Panels/LoggerConfigPanel.hpp"
#include "Panels/PilotConfigPanel.hpp"
#include "Panels/AirspaceWarningsConfigPanel.hpp"
#include "Panels/AirspaceConfigPanel.hpp"
#include "Panels/SiteConfigPanel.hpp"
#include "Panels/MapDisplayConfigPanel.hpp"
#include "Panels/WaypointDisplayConfigPanel.hpp"
#include "Panels/SymbolsConfigPanel.hpp"
#include "Panels/TerrainDisplayConfigPanel.hpp"
#include "Panels/GlideComputerConfigPanel.hpp"
#include "Panels/WindConfigPanel.hpp"
#include "Panels/SafetyFactorsConfigPanel.hpp"
#include "Panels/RouteConfigPanel.hpp"
#include "Panels/InterfaceConfigPanel.hpp"
#include "Panels/DisplayConfigPanel.hpp"
#include "Panels/LayoutConfigPanel.hpp"
#include "Panels/GaugesConfigPanel.hpp"
#include "Panels/MapOverlaysConfigPanel.hpp"
#include "Panels/TrafficConfigPanel.hpp"
#include "Panels/VarioConfigPanel.hpp"
#include "Panels/TaskRulesConfigPanel.hpp"
#include "Panels/TaskDefaultsConfigPanel.hpp"
#include "Panels/ScoringConfigPanel.hpp"
#include "Panels/InfoBoxesConfigPanel.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "UtilsSettings.hpp"
#include "Audio/Features.hpp"
#include "net/http/Features.hpp"

#ifdef HAVE_HTTP
#include "Panels/NOTAMConfigPanel.hpp"
#endif

#ifdef HAVE_PCM_PLAYER
#include "Panels/AudioConfigPanel.hpp"
#endif

#include "Tracking/Features.hpp"
#ifdef HAVE_SKYLINES_TRACKING
#include "Panels/SkyLinesConfigPanel.hpp"
#endif
#ifdef HAVE_LIVETRACK24
#include "Panels/LiveTrack24ConfigPanel.hpp"
#endif
#ifdef HAVE_TRACKING
#include "Panels/CloudConfigPanel.hpp"
#endif

#ifdef HAVE_HTTP
#include "Panels/WeatherConfigPanel.hpp"
#endif
#include "Panels/RaspConfigPanel.hpp"
#ifdef HAVE_PCMET
#include "Panels/PCMetConfigPanel.hpp"
#endif
#ifdef HAVE_HTTP
#include "Panels/XCThermConfigPanel.hpp"
#endif
#ifdef HAVE_HTTP
#include "Panels/SkySightConfigPanel.hpp"
#endif

#include "Panels/WeGlideConfigPanel.hpp"
#ifdef HAVE_HTTP
#include "Panels/SoaringScoringConfigPanel.hpp"
#endif
#include "Panels/NetworkConfigPanel.hpp"

#if defined(__linux__) && !defined(__ANDROID__) && !defined(KOBO)
#include "Panels/SystemdConfigPanel.hpp"
#ifdef USE_POLL_EVENT
#include "Panels/SystemToolsPanel.hpp"
#endif
#endif

#include <algorithm>
#include <cassert>

static unsigned current_page;

/** Menu page to show when Configuration opens. */
static std::unique_ptr<Widget> (*initial_page)() = nullptr;

// TODO: eliminate global variables
static ArrowPagerWidget *pager;
static TabMenuDisplay *menu_display;

static void
ShowDevices() noexcept
{
  if (backend_components != nullptr)
    ShowDeviceList(backend_components->devices.get());
}

/* Pages are created, and therefore saved, in the order of this table.
   Important: all pages after Units must not have data fields that are
   unit-dependent because they will be saved after their units may have
   changed.  The Units tile is therefore placed after every
   unit-dependent page. */

static void
ShowExportFlights() noexcept
{
  ShowExportFlightsDialog();
}

static constexpr TabMenuItem vario_items[] = {
  { .caption = N_("Glide Computer"), .Load = CreateGlideComputerConfigPanel },
  { .caption = N_("Safety Factors"), .Load = CreateSafetyFactorsConfigPanel },
  { .caption = N_("Wind"), .Load = CreateWindConfigPanel },
  { .caption = N_("Route"), .Load = CreateRouteConfigPanel },
  { nullptr }
};

static constexpr TabMenuItem display_items[] = {
  { .caption = N_("Screen"), .Load = CreateDisplayConfigPanel },
  { .caption = N_("Layout"), .Load = CreateLayoutConfigPanel },
  { .caption = N_("Pages"), .Load = CreatePagesConfigPanel },
  { .caption = N_("InfoBox Sets"), .Load = CreateInfoBoxesConfigPanel },
  { nullptr }
};

static constexpr TabMenuItem transfer_items[] = {
  { .caption = N_("Site Files"), .Load = CreateSiteConfigPanel },
  { .caption = NC_("Button", "Download manager"), .Action = ShowFileManager },
  { .caption = NC_("Button", "Export flights"),
    .Action = ShowExportFlights },
  { .caption = NC_("Button", "Import data"), .Action = ShowImportDataDialog },
  { .caption = NC_("Button", "Backup manager"),
    .Action = ShowBackupManagerDialog },
  { .caption = NC_("Button", "Advanced File Explorer"),
    .Action = ShowAdvancedFileExplorerDialog },
  { nullptr }
};

static constexpr TabMenuItem map_items[] = {
  { .caption = N_("Orientation"), .Load = CreateMapDisplayConfigPanel },
  { .caption = N_("Overlays"), .Load = CreateMapOverlaysConfigPanel },
  { .caption = N_("Terrain"), .Load = CreateTerrainDisplayConfigPanel },
  { nullptr }
};

static constexpr TabMenuItem weather_items[] = {
#ifdef HAVE_PCMET
  { .caption = "Flugwetter (pc_met)", .Load = CreatePCMetConfigPanel },
#endif
  { .caption = "RASP", .Load = CreateRaspConfigPanel },
#ifdef HAVE_HTTP
  { .caption = "SkySight", .Load = CreateSkySightConfigPanel },
  { .caption = N_("Thermal Information Map"),
    .Load = CreateWeatherConfigPanel },
  { .caption = "XC Therm", .Load = CreateXCThermConfigPanel },
#endif
  { nullptr }
};

static constexpr TabMenuItem airspace_items[] = {
  { .caption = N_("Airspace"), .Load = CreateAirspaceConfigPanel },
#ifdef HAVE_HTTP
  { .caption = NC_("Setting", "NOTAM"), .Load = CreateNOTAMConfigPanel },
#endif
  { nullptr }
};

static constexpr TabMenuItem thermal_items[] = {
  { .caption = N_("Thermal Assistant"), .Load = CreateGaugesConfigPanel },
  { .caption = N_("Vario"), .Load = CreateVarioConfigPanel },
  { nullptr }
};

/* the pages of the original Graphics sub-menu of the LX9000 */
static constexpr TabMenuItem graphics_items[] = {
  { .caption = N_("Map and Terrain"), .children = map_items,
    .icon = "lx_map" },
  { .caption = N_("Weather"), .children = weather_items,
    .icon = "lx_weather" },
  { .caption = N_("Airspaces and NOTAMs"), .children = airspace_items,
    .icon = "lx_airspace" },
  { .caption = N_("Waypoints and Airports"),
    .Load = CreateWaypointDisplayConfigPanel,
    .icon = "lx_waypoints" },
  { .caption = N_("Glider and Track"), .Load = CreateSymbolsConfigPanel,
    .icon = "lx_glider" },
  { .caption = N_("Thermal Mode"), .children = thermal_items,
    .icon = "lx_thermal" },
  { .caption = N_("FLARM"), .Load = CreateTrafficConfigPanel,
    .icon = "lx_flarm" },
  { nullptr }
};

static constexpr TabMenuItem task_items[] = {
  { .caption = N_("Turnpoint Types"), .Load = CreateTaskDefaultsConfigPanel },
  { .caption = N_("Task Rules"), .Load = CreateTaskRulesConfigPanel },
  { nullptr }
};

static constexpr TabMenuItem hardware_items[] = {
  { .caption = N_("Devices"), .Action = ShowDevices },
  { .caption = N_("Network"), .Load = CreateNetworkConfigPanel },
  { .caption = NC_("Setting", "Time"), .Load = CreateTimeConfigPanel },
#if defined(__linux__) && !defined(__ANDROID__) && !defined(KOBO)
  { .caption = N_("System Services"), .Load = CreateSystemdConfigPanel },
#ifdef USE_POLL_EVENT
  { .caption = N_("System Tools"), .Load = CreateSystemToolsPanel,
    .IsAvailable = HasSystemTools },
#endif
#endif
  { nullptr }
};

static constexpr TabMenuItem pilot_items[] = {
  { .caption = N_("Profiles"), .Action = ProfileListDialog },
  { .caption = N_("Pilots"), .Load = CreatePilotConfigPanel },
  { nullptr }
};

static constexpr TabMenuItem service_items[] = {
#ifdef HAVE_LIVETRACK24
  { .caption = "LiveTrack24", .Load = CreateLiveTrack24ConfigPanel },
#endif
#ifdef HAVE_SKYLINES_TRACKING
  { .caption = "SkyLines", .Load = CreateSkyLinesConfigPanel },
#endif
  { .caption = "WeGlide", .Load = CreateWeGlideConfigPanel },
#ifdef HAVE_HTTP
  { .caption = "SoaringScoring", .Load = CreateSoaringScoringConfigPanel },
#endif
#ifdef HAVE_TRACKING
  { .caption = "XCSoar Cloud", .Load = CreateCloudConfigPanel },
#endif
  { nullptr }
};

/**
 * The tiles of the LXNAV LX9000 "Setup" menu, in the order of the
 * original.
 */
static constexpr TabMenuItem main_menu_items[] = {
  { .caption = N_("QNH and RES"), .Action = dlgBasicSettingsShowModal,
    .icon = "lx_qnhres" },
  { .caption = N_("Flight Recorder"), .Load = CreateLoggerConfigPanel,
    .icon = "lx_loger" },
  { .caption = N_("Weight and Balance"), .Action = dlgBasicSettingsShowModal,
    .icon = "lx_weightbalance" },
  { .caption = N_("Vario Parameters"), .children = vario_items,
    .icon = "lx_varioset" },
  { .caption = N_("Display"), .children = display_items,
    .icon = "lx_display" },
  { .caption = N_("Files and Transfer"), .children = transfer_items,
    .icon = "lx_transfer" },
  { .caption = N_("Graphics"), .children = graphics_items,
    .icon = "lx_graphics" },
#ifdef HAVE_PCM_PLAYER
  /* Before Units: audio vario deadband uses vertical-speed units. */
  { .caption = N_("Sounds"), .Load = CreateAudioConfigPanel,
    .icon = "lx_sounds" },
#endif
  { .caption = N_("Observation Zones"), .children = task_items,
    .icon = "lx_oz" },
  { .caption = N_("Optimization"), .Load = CreateScoringConfigPanel,
    .icon = "lx_optimization" },
  { .caption = N_("Warnings"), .Load = CreateAirspaceWarningsConfigPanel,
    .icon = "lx_warnings" },
  { .caption = N_("Units"), .Load = CreateUnitsConfigPanel,
    .icon = "lx_units" },
  { .caption = N_("Hardware"), .children = hardware_items,
    .icon = "lx_hardware" },
  { .caption = N_("Polar and Glider"), .Action = dlgPlanesShowModal,
    .icon = "lx_polar" },
  { .caption = N_("Profiles and Pilots"), .children = pilot_items,
    .icon = "lx_pilots" },
  { .caption = NC_("Menu", "Services"), .children = service_items,
    .icon = "lx_cloudservices" },
  { .caption = N_("Language"), .Load = CreateInterfaceConfigPanel,
    .icon = "lx_lang" },
  { .caption = N_("Password"), .Action = ChangeProfilePassword,
    .icon = "lx_pwd" },
  { nullptr }
};

static void
OnUserLevel(bool expert) noexcept;

class ConfigurationExtraButtons final
  : public NullWidget {
  /** Landscape stacks up to this many page actions above Previous. */
  static constexpr unsigned MAX_EXTRA_BUTTONS = 4;

  const DialogLook &look;

  CheckBoxControl expert;
  Button buttons[MAX_EXTRA_BUTTONS];
  bool borrowed[MAX_EXTRA_BUTTONS] = {};

  PixelRect position{};

  /** Taller than the single portrait row: the landscape column. */
  bool IsStack(const PixelRect &rc) const noexcept {
    return rc.GetHeight() > ::Layout::GetMaximumControlHeight();
  }

  void Apply(const PixelRect &rc) noexcept;

public:
  explicit ConfigurationExtraButtons(const DialogLook &_look) noexcept
    :look(_look) {}

  Button &GetButton(unsigned number) noexcept {
    assert(number >= 1 && number <= MAX_EXTRA_BUTTONS);
    return buttons[number - 1];
  }

  CheckBoxControl &GetExpert() noexcept {
    return expert;
  }

  void Borrow(unsigned number, const char *caption,
              std::function<void()> callback) noexcept {
    assert(number >= 1 && number <= MAX_EXTRA_BUTTONS);
    const unsigned i = number - 1;
    borrowed[i] = true;
    buttons[i].SetMenuCaption(look.button, caption);
    buttons[i].SetCallback(std::move(callback));
    if (expert.IsDefined())
      Apply(position);
  }

  void Return(unsigned number) noexcept {
    assert(number >= 1 && number <= MAX_EXTRA_BUTTONS);
    const unsigned i = number - 1;
    borrowed[i] = false;
    if (buttons[i].IsDefined())
      buttons[i].Hide();
  }

protected:
  /* virtual methods from Widget */
  PixelSize GetMinimumSize() const noexcept override {
    const unsigned height = ::Layout::GetMaximumControlHeight();
    unsigned width = CheckBoxControl::GetMinimumWidth(look, height,
                                                      _("Expert"));
    const ButtonLook &button_look = look.button;
    const char *const captions[] = {
      C_("Button", "Add"),
      C_("Button", "Delete"),
      _("Turn off"),
      _("Restart"),
    };
    for (const char *caption : captions)
      width = std::max(width,
                       TextButtonRenderer::GetMinimumButtonWidth(
                         button_look, caption));
    return { width, height * 3 };
  }

  void Prepare(ContainerWindow &parent,
               const PixelRect &rc) noexcept override {
    position = rc;

    expert.CreateInDialogForm(parent, look, _("Expert"), rc,
                              [](bool value){ OnUserLevel(value); });

    WindowStyle style;
    style.Hide();
    style.TabStop();

    /* High index first so child order matches the stack, top to
       bottom.  Up then walks v → ^ → Delete → Add. */
    for (unsigned n = MAX_EXTRA_BUTTONS; n > 0; --n)
      buttons[n - 1].Create(parent, look.button, "", rc, style);

    Apply(rc);
  }

  void Show(const PixelRect &rc) noexcept override {
    expert.SetState(CommonInterface::GetUISettings().dialog.expert);
    expert.Show();
    Apply(rc);
  }

  void Hide() noexcept override {
    expert.FastHide();
    for (auto &button : buttons)
      button.FastHide();
  }

  void Move(const PixelRect &rc) noexcept override {
    Apply(rc);
  }
};

void
ConfigurationExtraButtons::Apply(const PixelRect &rc) noexcept
{
  if (!expert.IsDefined())
    return;

  position = rc;
  if (rc.top >= rc.bottom || rc.left >= rc.right)
    return;

  const unsigned h = ::Layout::GetMaximumControlHeight();
  const bool stack = IsStack(rc);
  bool placed[MAX_EXTRA_BUTTONS] = {};

  auto place = [&](unsigned n, PixelRect br) {
    if (br.top >= br.bottom || br.left >= br.right)
      return;
    buttons[n].Move(br);
    placed[n] = true;
  };

  if (!stack) {
    /* Portrait: Expert | button 2 | button 1. */
    PixelRect expert_rc = rc, middle = rc, right = rc;
    expert_rc.right = middle.left =
      (unsigned)(rc.left * 2 + rc.right) / 3;
    middle.right = right.left =
      (unsigned)(rc.left + rc.right * 2) / 3;
    if (expert_rc.left < expert_rc.right)
      expert.Move(expert_rc);
    place(1, middle);
    place(0, right);
  } else {
    PixelRect expert_rc = rc;
    expert_rc.bottom = expert_rc.top + (int)h;
    if (expert_rc.bottom > rc.bottom)
      expert_rc.bottom = rc.bottom;
    if (expert_rc.top < expert_rc.bottom)
      expert.Move(expert_rc);

    unsigned count = 0;
    for (unsigned n = 0; n < MAX_EXTRA_BUTTONS; ++n)
      if (borrowed[n])
        ++count;

    const int room = rc.bottom - expert_rc.bottom;
    unsigned slot = h;
    if (count > 0 && room > 0 && room < (int)(count * h))
      slot = std::max(1u, unsigned(room) / count);

    int bottom = rc.bottom;
    for (unsigned n = 0; n < MAX_EXTRA_BUTTONS; ++n) {
      if (!borrowed[n] || bottom <= expert_rc.bottom)
        continue;

      PixelRect br = rc;
      br.bottom = bottom;
      br.top = bottom - (int)slot;
      if (br.top < expert_rc.bottom)
        br.top = expert_rc.bottom;
      bottom = br.top;
      place(n, br);
    }
  }

  if (!expert.IsVisible())
    return;

  for (unsigned n = 0; n < MAX_EXTRA_BUTTONS; ++n) {
    if (!buttons[n].IsDefined())
      continue;

    /* Portrait has room for the original two buttons only. */
    if (borrowed[n] && placed[n] && (stack || n < 2))
      buttons[n].Show();
    else
      buttons[n].Hide();
  }
}

bool
ConfigPanel::HasExtraButtons() noexcept
{
  return pager != nullptr;
}

Button &
ConfigPanel::BorrowExtraButton(unsigned i, const char *caption,
                               std::function<void()> callback) noexcept
{
  assert(pager != nullptr);

  auto &extra = (ConfigurationExtraButtons &)pager->GetExtra();
  extra.Borrow(i, caption, std::move(callback));
  return extra.GetButton(i);
}

void
ConfigPanel::ReturnExtraButton(unsigned i)
{
  assert(pager != nullptr);

  auto &extra = (ConfigurationExtraButtons &)pager->GetExtra();
  extra.Return(i);
}

CheckBoxControl &
ConfigPanel::GetExpertCheckbox() noexcept
{
  assert(pager != nullptr);

  auto &extra = (ConfigurationExtraButtons &)pager->GetExtra();
  return extra.GetExpert();
}

static void
OnUserLevel(bool expert) noexcept
{
  CommonInterface::SetUISettings().dialog.expert = expert;

  /* Keep Profile I/O out of this checkbox callback (pager is mid-
     relayout). Persist UserLevel when the dialog closes instead. */

  /* force layout update */
  pager->PagerWidget::Move(pager->GetPosition());
}

/**
 * Close on the main grid commits (mrOK).  On a settings page or a
 * sub-grid, go back (Back).
 */
static void
OnCloseClicked(WidgetDialog &dialog)
{
  if (pager->GetCurrentIndex() != 0)
    pager->ClickPage(0);
  else if (!menu_display->LeaveSubGrid())
    dialog.SetModalResult(mrOK);
}

/** Show the title and the Close/Back button of the current page. */
static void
UpdateChrome(WidgetDialog &dialog, TabMenuDisplay &menu)
{
  char buffer[128];
  const char *caption = menu.GetCaption(buffer, ARRAY_SIZE(buffer));
  if (caption == nullptr)
    caption = _("Configuration");
  dialog.SetCaption(caption);

  pager->SetCloseButtonCaption(pager->GetCurrentIndex() == 0 &&
                               !menu.IsSubGrid()
                               ? _("Close")
                               : _("Back"));
}

static void
OnPageFlipped(WidgetDialog &dialog, TabMenuDisplay &menu)
{
  menu.OnPageFlipped();
  UpdateChrome(dialog, menu);
}

void
dlgConfigurationShowPage(std::unique_ptr<Widget> (*page)())
{
  assert(page != nullptr);

  /* Already inside Configuration: this is one of its pages. */
  if (pager != nullptr) {
    pager->SetCurrent(menu_display->FindPage(page), false);
    return;
  }

  initial_page = page;
  AtScopeExit() {
    initial_page = nullptr;
  };
  SystemConfiguration();
}

void dlgConfigurationShowModal()
{
  const DialogLook &look = UIGlobals::GetDialogLook();

  /* Destroyed after the dialog, so Hide() can still return borrowed
     buttons. Left set, the pointer dangles and a later standalone
     panel crashes in BorrowExtraButton(). */
  AtScopeExit() {
    pager = nullptr;
    menu_display = nullptr;
  };

  WidgetDialog dialog(WidgetDialog::Full{}, UIGlobals::GetMainWindow(),
                      look, _("Configuration"));

  pager = new ArrowPagerWidget(look.button,
                               [&dialog](){ OnCloseClicked(dialog); },
                               std::make_unique<ConfigurationExtraButtons>(look));

  auto _menu = std::make_unique<TabMenuDisplay>(*pager, look);
  auto &menu = *_menu;
  menu_display = &menu;
  pager->Add(std::make_unique<CreateWindowWidget>([&_menu](ContainerWindow &parent,
                                                           const PixelRect &rc,
                                                           WindowStyle style) {
    style.TabStop();
    _menu->Create(parent, rc, style);
    return std::move(_menu);
  }));

  menu.InitMenu(main_menu_items, C_("Menu", "Setup"));

  /* restore last selected menu item */
  menu.SetCursor(current_page);

  if (initial_page != nullptr) {
    pager->SetCurrent(menu.FindPage(initial_page), false);
  }

  pager->SetPageFlippedCallback([&dialog, &menu](){
    OnPageFlipped(dialog, menu);
  });

  menu.SetLevelCallback([&dialog, &menu](){
    UpdateChrome(dialog, menu);
  });

  dialog.FinishPreliminary(pager);

  if (initial_page != nullptr) {
    OnPageFlipped(dialog, menu);
  }

  /* Esc on a settings panel returns to the menu (same as Back); in a
     sub-grid it returns to the main grid; there, leave Esc to cancel
     the dialog. */
  dialog.SetKeyDownFunction([&dialog, &menu](unsigned key_code) {
    if (key_code != KEY_ESCAPE)
      return false;

    if (pager->GetCurrentIndex() == 0)
      return menu.LeaveSubGrid();

    OnCloseClicked(dialog);
    return true;
  });

  const int result = dialog.ShowModal();

  /* save page number for next time this dialog is opened */
  current_page = menu.GetCursor();

  /* Persist Expert only on OK. Missing UserLevel means beginner —
     write "1" when enabling Expert; remove the key when returning to
     beginner (do not leave UserLevel=0 cruft) (#1793). */
  bool expert_changed = false;
  if (result == mrOK) {
    const bool expert = CommonInterface::GetUISettings().dialog.expert;
    if (expert) {
      bool profile_expert = false;
      Profile::Get(ProfileKeys::UserLevel, profile_expert);
      if (!profile_expert) {
        Profile::Set(ProfileKeys::UserLevel, true);
        expert_changed = true;
      }
    } else if (Profile::Exists(ProfileKeys::UserLevel)) {
      Profile::Remove(ProfileKeys::UserLevel);
      expert_changed = true;
    }
  }

  if (dialog.GetChanged() || expert_changed)
    Profile::Save();
}
