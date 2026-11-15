// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "StartupDialog.hpp"
#include "Error.hpp"
#include "Form/DataField/File.hpp"
#include "Language/Language.hpp"
#include "LocalPath.hpp"
#include "LogFile.hpp"
#include "Profile/Profile.hpp"
#include "Repository/FileType.hpp"
#include "ProfileListDialog.hpp"
#include "ProfilePasswordDialog.hpp"
#include "ProfileScreen.hpp"
#include "UIGlobals.hpp"
#include "Widget/WindowWidget.hpp"
#include "WidgetDialog.hpp"
#include "system/FileUtil.hpp"
#include "ui/event/KeyCode.hpp"

static bool
SelectProfile(Path path) noexcept
{
  try {
    if (!CheckProfilePasswordResult(CheckProfileFilePassword(path)))
      return false;
  } catch (...) {
    ShowError(std::current_exception(), _("Password"));
    return false;
  }

  Profile::SetFiles(path);

  if (RelativePath(path) == nullptr)
    /* When a profile from a secondary data path is used, this path
       becomes the primary data path */
    SetPrimaryDataPath(path.GetParent());

  File::Touch(path);
  return true;
}

/** Hosts the #ProfileScreen and maps keys and buttons to the dialog. */
class StartupWidget final : public WindowWidget {
  WidgetDialog &dialog;
  FileDataField &df;

public:
  StartupWidget(WidgetDialog &_dialog, FileDataField &_df) noexcept
    :dialog(_dialog), df(_df) {}

  /* virtual methods from class Widget */
  void Prepare(ContainerWindow &parent,
               const PixelRect &rc) noexcept override;
  bool Save(bool &changed) noexcept override;
  bool KeyPress(unsigned key_code) noexcept override;

private:
  ProfileScreen &GetScreen() noexcept {
    return static_cast<ProfileScreen &>(GetWindow());
  }

  void PickProfile() noexcept;
};

void
StartupWidget::PickProfile() noexcept
{
  const auto path = SelectProfileDialog(df.GetValue());
  if (path == nullptr)
    return;

  df.ForceModify(path);
  GetScreen().OnSelectionChanged();
}

void
StartupWidget::Prepare(ContainerWindow &parent,
                       const PixelRect &rc) noexcept
{
  WindowStyle style;
  style.Hide();
  style.ControlParent();

  auto screen = std::make_unique<ProfileScreen>(
    df, [this](){ PickProfile(); },
    dialog.MakeModalResultCallback(mrCancel),
    dialog.MakeModalResultCallback(mrOK));
  screen->Create(parent, rc, style);
  SetWindow(std::move(screen));
}

bool
StartupWidget::KeyPress(unsigned key_code) noexcept
{
  /* like the LX knob: the cursor keys change the profile directly */
  switch (key_code) {
  case KEY_UP:
  case KEY_LEFT:
    GetScreen().Step(-1);
    return true;

  case KEY_DOWN:
  case KEY_RIGHT:
    GetScreen().Step(1);
    return true;

  case KEY_RETURN:
    dialog.SetModalResult(mrOK);
    return true;

  default:
    return false;
  }
}

bool
StartupWidget::Save(bool &changed) noexcept
{
  if (!SelectProfile(df.GetValue()))
    return false;

  changed = true;

  return true;
}

bool
dlgStartupShowModal() noexcept
{
  LogString("Startup dialog");

  /* scan all profile files */
  auto *dff = new FileDataField();
  dff->SetFileType(FileType::PROFILE);
  dff->ScanDirectoryTop(GetFileTypePatterns(FileType::PROFILE));

  if (dff->GetNumFiles() == 1) {
    /* skip this dialog if there is only one */
    const auto path = dff->GetValue();
    if (ProfileFileHasPassword(path) == TriState::FALSE &&
        SelectProfile(path)) {
      delete dff;
      return true;
    }
  } else if (dff->GetNumFiles() == 0) {
    /* no profile exists yet: create default profile */
    Profile::SetFiles(nullptr);
    return true;
  }

  /* preselect the most recently used profile */
  unsigned best_index = 0;
  std::chrono::system_clock::time_point best_timestamp =
    std::chrono::system_clock::time_point::min();
  unsigned length = dff->size();

  for (unsigned i = 0; i < length; ++i) {
    const auto path = Path(dff->GetItem(i).path);
    const auto timestamp = File::GetLastModification(path);
    if (timestamp > best_timestamp) {
      best_timestamp = timestamp;
      best_index = i;
    }
  }

  dff->SetIndex(best_index);

  /* show the dialog */
  TWidgetDialog<StartupWidget> dialog(WidgetDialog::Full{},
                                      UIGlobals::GetMainWindow(),
                                      UIGlobals::GetDialogLook(), nullptr);
  dialog.SetWidget(dialog, *dff);

  return dialog.ShowModal() == mrOK;
}
