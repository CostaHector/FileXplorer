#include "ShadowRenamerActions.h"
#include "RenamerKey.h"
#include "Configuration.h"
#include "PublicMacro.h"
#include "ImageTool.h"
#include "StyleSheet.h"
#include <QToolBar>

ShadowRenamerActions& ShadowRenamerActions::GetInst() {
  static ShadowRenamerActions ins;
  return ins;
}

ShadowRenamerActions::ShadowRenamerActions(QObject* parent)
  : QObject{parent},
    m_bOpenSourceOnDoubleClick{getConfig(RenamerKey::OPEN_SOURCE_ON_DOUBLE_CLICK_SW).toBool()}
{
  CREATE_STAGING_FILES = new (std::nothrow) QAction{QIcon{":img/SHADOW_CREATE_STAGING_FILES"}, tr("Create Staging Files"), this};
  CHECK_NULLPTR_RETURN_VOID(CREATE_STAGING_FILES);
  CREATE_STAGING_FILES->setToolTip("Create staging files for all video files in the source folder. "
                                   "These staging files mirror the folder structure and store the original "
                                   "filename and sync status.");

  SYNC_STAGING_TO_VIDEO = new (std::nothrow) QAction{QIcon{":img/SHADOW_SYNC_STAGING_TO_VIDEO"}, tr("Apply Staging to Videos"), this};
  CHECK_NULLPTR_RETURN_VOID(SYNC_STAGING_TO_VIDEO);
  SYNC_STAGING_TO_VIDEO->setToolTip("Apply the renames from staging files back to the original video folder. "
                                    "Only files whose staging names differ from their stored original names "
                                    "will be renamed.");

  SHOW_STAGING_STATISTICS = new (std::nothrow) QAction{QIcon{":img/SHADOW_STAGING_STATISTICS"}, tr("Show Staging Statistics"), this};
  CHECK_NULLPTR_RETURN_VOID(SHOW_STAGING_STATISTICS);
  SHOW_STAGING_STATISTICS->setToolTip("Show a summary of all staging files, grouped by sync status: "
                                      "pending, already synced, no sync needed, and no corresponding file.");

  RECYCLE_SYNCED_STAGING_FILE = new (std::nothrow) QAction{tr("Recycle Synced Staging Files"), this};
  CHECK_NULLPTR_RETURN_VOID(RECYCLE_SYNCED_STAGING_FILE);
  RECYCLE_SYNCED_STAGING_FILE->setToolTip("Recycle all staging files that have already been synced back to the "
                                         "video folder. This keeps the staging folder clean.");

  RECYCLE_NO_NEED_SYNC_FILE = new (std::nothrow) QAction{tr("Recycle No-Sync-Needed Staging Files"), this};
  CHECK_NULLPTR_RETURN_VOID(RECYCLE_NO_NEED_SYNC_FILE);
  RECYCLE_NO_NEED_SYNC_FILE->setToolTip("Recycle all staging files that need no sync. "
                                       "These are files whose staging path still matches their stored original "
                                       "path, meaning the user has not renamed them yet.");

  OPEN_SOURCE_ON_DOUBLE_CLICK = new (std::nothrow) QAction{tr("Double-Click Opens Original"), this};
  CHECK_NULLPTR_RETURN_VOID(OPEN_SOURCE_ON_DOUBLE_CLICK);
  OPEN_SOURCE_ON_DOUBLE_CLICK->setCheckable(true);
  OPEN_SOURCE_ON_DOUBLE_CLICK->setChecked(m_bOpenSourceOnDoubleClick);
  OPEN_SOURCE_ON_DOUBLE_CLICK->setToolTip("When enabled, double-clicking a staging file opens the corresponding original video file. "
                                          "When disabled, double-clicking opens the staging file itself.");
  connect(OPEN_SOURCE_ON_DOUBLE_CLICK, &QAction::toggled, this, [this](bool bOpenSourceSw) {
    m_bOpenSourceOnDoubleClick = bOpenSourceSw;
    setConfig(RenamerKey::OPEN_SOURCE_ON_DOUBLE_CLICK_SW, bOpenSourceSw);
  });
}

QWidget* ShadowRenamerActions::GetShadowFilesToolBar(QWidget* notNullParent) const {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  auto* shadowFilesTB = new (std::nothrow) QToolBar{"Shadow Files", notNullParent};
  CHECK_NULLPTR_RETURN_NULLPTR(shadowFilesTB);
  shadowFilesTB->setOrientation(Qt::Orientation::Vertical);
  shadowFilesTB->addAction(CREATE_STAGING_FILES);
  shadowFilesTB->addAction(SYNC_STAGING_TO_VIDEO);
  shadowFilesTB->addAction(SHOW_STAGING_STATISTICS);
  shadowFilesTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  shadowFilesTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  SetLayoutAlightment(shadowFilesTB->layout(), Qt::AlignmentFlag::AlignLeft);
  return shadowFilesTB;
}

QWidget* ShadowRenamerActions::GetShadowFilesRecycleToolBar(QWidget* notNullParent) const {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  auto* recycleItemsTB = new (std::nothrow) QToolBar{"Shadow Files", notNullParent};
  CHECK_NULLPTR_RETURN_NULLPTR(recycleItemsTB);
  recycleItemsTB->setOrientation(Qt::Orientation::Vertical);
  recycleItemsTB->addAction(RECYCLE_SYNCED_STAGING_FILE);
  recycleItemsTB->addAction(RECYCLE_NO_NEED_SYNC_FILE);
  recycleItemsTB->addAction(OPEN_SOURCE_ON_DOUBLE_CLICK);
  recycleItemsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  recycleItemsTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  SetLayoutAlightment(recycleItemsTB->layout(), Qt::AlignmentFlag::AlignLeft);
  return recycleItemsTB;
}