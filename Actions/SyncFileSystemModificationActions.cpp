#include "SyncFileSystemModificationActions.h"
#include "SyncModifiyFileSystem.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "StyleSheet.h"

SyncFileSystemModificationActions& g_syncFileSystemModificationActions() {
  static SyncFileSystemModificationActions ins;
  return ins;
}

SyncFileSystemModificationActions::SyncFileSystemModificationActions(QObject* /*parent*/) {
  _SYNC_MOD_SWITCH = new (std::nothrow) QAction{QIcon(":img/SYNC_MODIFICATION_SWITCH"), "Sync", this};
  _SYNC_MOD_SWITCH->setCheckable(true);
  _SYNC_MOD_SWITCH->setToolTip(
      "Modification on basic path will also syncronized to destination path,"
      "including\n1.file rename\n2.file move/into into\n3.file recycle\n4.file delete\nWarning: When copy/cut items out of these 2 paths, only first commands will succeed.");

  _SYNC_REVERSE_SWITCH = new (std::nothrow) QAction{QIcon(":img/SYNC_REVERSE_SWITCH"), "reverse back", this};
  _SYNC_REVERSE_SWITCH->setCheckable(true);
  _SYNC_REVERSE_SWITCH->setToolTip("Also sync modification reverse back to basic path");

  SyncModifiyFileSystem::LoadFromMemory();
}

QToolBar* SyncFileSystemModificationActions::GetSyncSwitchToolbar(QWidget* parent) {
  _SYNC_MOD_SWITCH->setChecked(SyncModifiyFileSystem::m_syncOperationSw);
  _SYNC_REVERSE_SWITCH->setChecked(SyncModifiyFileSystem::m_syncBackSw);

  QToolBar* syncSwTb = new (std::nothrow) QToolBar{"Sync Switch", parent};
  CHECK_NULLPTR_RETURN_NULLPTR(syncSwTb)
  syncSwTb->addAction(_SYNC_MOD_SWITCH);
  syncSwTb->setStyleSheet("QToolBar { max-width: 256px; }");
  syncSwTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return syncSwTb;
}

QToolBar* SyncFileSystemModificationActions::GetSyncPathToolbar(QWidget* parent) {
  QToolBar* syncTb = new (std::nothrow) QToolBar{"Sync Path Widget", parent};
    CHECK_NULLPTR_RETURN_NULLPTR(syncTb)

    _BASIC_PATH = new (std::nothrow) QLineEdit{SyncModifiyFileSystem::m_basicPath, syncTb};
    CHECK_NULLPTR_RETURN_NULLPTR(_BASIC_PATH)
    _BASIC_PATH->setClearButtonEnabled(true);
    _BASIC_PATH->setToolTip("Basic path");

    _SYNC_TO_PATH = new (std::nothrow) QLineEdit{SyncModifiyFileSystem::m_syncToPath, syncTb};
    CHECK_NULLPTR_RETURN_NULLPTR(_SYNC_TO_PATH)
    _SYNC_TO_PATH->setClearButtonEnabled(true);
    _SYNC_TO_PATH->setToolTip("Path modification will also synchronized to");

    syncTb->addAction(_SYNC_REVERSE_SWITCH);
    syncTb->addWidget(_BASIC_PATH);
    syncTb->addWidget(_SYNC_TO_PATH);
    syncTb->setOrientation(Qt::Orientation::Vertical);
    syncTb->setStyleSheet("QToolBar { max-width: 512px; }");
    syncTb->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
    syncTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);

    const bool syncSwOn{SyncModifiyFileSystem::m_syncOperationSw};
    _BASIC_PATH->setEnabled(syncSwOn);
    _SYNC_TO_PATH->setEnabled(syncSwOn);
    _SYNC_REVERSE_SWITCH->setEnabled(syncSwOn);

    connect(_SYNC_MOD_SWITCH, &QAction::triggered, this, [this](const bool sw) {
      SyncModifiyFileSystem::SetSyncOperationSwitch(sw);
      _BASIC_PATH->setEnabled(sw);
      _SYNC_TO_PATH->setEnabled(sw);
      _SYNC_REVERSE_SWITCH->setEnabled(sw);
    });

    connect(_SYNC_REVERSE_SWITCH, &QAction::toggled, &SyncModifiyFileSystem::SetSyncReverseBackSwitch);

    connect(_BASIC_PATH, &QLineEdit::returnPressed, this, [this]() {
      const QString& basicPath{_BASIC_PATH->text()};
      const bool setResult = SyncModifiyFileSystem::SetBasicPath(basicPath);
      if (!setResult) {
        LOG_ERR_NP("Cannot set basic path:", basicPath);
      } else {
        LOG_OK_NP("Set basic path", basicPath);
      }
    });

    connect(_SYNC_TO_PATH, &QLineEdit::returnPressed, this, [this]() {
      const QString& syncToPath{_SYNC_TO_PATH->text()};
      const bool setResult = SyncModifiyFileSystem::SetSynchronizedToPaths(syncToPath);
      if (!setResult) {
        LOG_ERR_NP("Cannot set operation sync to path:", syncToPath);
      } else {
        LOG_OK_NP("Set operation sync to path:", syncToPath);
      }
    });

    return syncTb;
  }
