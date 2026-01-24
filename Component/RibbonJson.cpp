#include "RibbonJson.h"
#include "PublicMacro.h"
#include "PublicTool.h"
#include "StyleSheet.h"
#include "PublicVariable.h"
#include "JsonActions.h"
#include "ViewActions.h"

RibbonJson::RibbonJson(const QString& title, QWidget* parent) //
  : QToolBar{title, parent}                                   //
{
  auto& inst = g_JsonActions();

  mSyncCacheFileSystemTb = new (std::nothrow) QToolBar{"Sync cache/file system", this};
  CHECK_NULLPTR_RETURN_VOID(mSyncCacheFileSystemTb);
  mSyncCacheFileSystemTb->addAction(inst._SYNC_NAME_FIELD_BY_FILENAME);
  mSyncCacheFileSystemTb->addAction(inst._RELOAD_JSON_FROM_FROM_DISK);
  mSyncCacheFileSystemTb->addAction(inst._EXPORT_CAST_STUDIO_TO_DICTION);
  mSyncCacheFileSystemTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mSyncCacheFileSystemTb->setOrientation(Qt::Orientation::Vertical);
  mSyncCacheFileSystemTb->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  mSyncCacheFileSystemTb->setStyleSheet("QToolBar { max-width: 256px; }");
  SetLayoutAlightment(mSyncCacheFileSystemTb->layout(), Qt::AlignmentFlag::AlignLeft);

  mFieldCaseOperTb = new (std::nothrow) QToolBar{"Field case Control", this};
  CHECK_NULLPTR_RETURN_VOID(mFieldCaseOperTb);
  mFieldCaseOperTb->addAction(inst._CAPITALIZE_FIRST_LETTER_OF_EACH_WORD);
  mFieldCaseOperTb->addAction(inst._LOWER_ALL_WORDS);
  mFieldCaseOperTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mFieldCaseOperTb->setOrientation(Qt::Orientation::Vertical);
  mFieldCaseOperTb->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  mFieldCaseOperTb->setStyleSheet("QToolBar { max-width: 256px; }");
  SetLayoutAlightment(mFieldCaseOperTb->layout(), Qt::AlignmentFlag::AlignLeft);

  mInitFormatStudioCastField = new (std::nothrow) QToolBar{"Init/Hint/Format studio/cast", this};
  CHECK_NULLPTR_RETURN_VOID(mInitFormatStudioCastField);
  mInitFormatStudioCastField->addAction(inst._INIT_STUDIO_CAST_FIELD);
  mInitFormatStudioCastField->addAction(inst._FORMAT_STUDIO_CAST_FIELD);
  mInitFormatStudioCastField->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mInitFormatStudioCastField->setOrientation(Qt::Orientation::Vertical);
  mInitFormatStudioCastField->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  mInitFormatStudioCastField->setStyleSheet("QToolBar { max-width: 256px; }");
  SetLayoutAlightment(mInitFormatStudioCastField->layout(), Qt::AlignmentFlag::AlignLeft);

  mInferStudioCastFromSelection = new (std::nothrow) QToolBar{"Infer studio/cast", this};
  CHECK_NULLPTR_RETURN_VOID(mInferStudioCastFromSelection);
  mInferStudioCastFromSelection->addAction(inst._INFER_CAST_STUDIO);
  mInferStudioCastFromSelection->addAction(inst._INFER_CAST_FROM_SELECTION);
  mInferStudioCastFromSelection->addAction(inst._INFER_CAST_FROM_UPPERCASE_SELECTION);
  mInferStudioCastFromSelection->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mInferStudioCastFromSelection->setOrientation(Qt::Orientation::Vertical);
  mInferStudioCastFromSelection->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  mInferStudioCastFromSelection->setStyleSheet("QToolBar { max-width: 256px; }");
  SetLayoutAlightment(mInferStudioCastFromSelection->layout(), Qt::AlignmentFlag::AlignLeft);

  mStudioMenu = new (std::nothrow) MenuToolButton{inst.m_studioOperationList,
                                                  QToolButton::ToolButtonPopupMode::MenuButtonPopup,
                                                  Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
                                                  IMAGE_SIZE::TABS_ICON_IN_MENU_24,
                                                  this};
  mStudioMenu->SetCaption(QIcon(":img/STUDIO"), tr("Studio"), "Set/Clear studio");
  mStudioMenu->InitDefaultActionFromQSetting(JsonOpMemoryKey::STUDIO_OPERATION(), true);

  mCastMenu = new (std::nothrow) MenuToolButton{inst.m_castOperationList,
                                                QToolButton::ToolButtonPopupMode::MenuButtonPopup,
                                                Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
                                                IMAGE_SIZE::TABS_ICON_IN_MENU_48,
                                                this};
  mCastMenu->InitDefaultActionFromQSetting(JsonOpMemoryKey::CAST_OPERATION(), true);

  mTagsMenu = new (std::nothrow) MenuToolButton{inst.m_tagsOperationList,
                                                QToolButton::ToolButtonPopupMode::MenuButtonPopup,
                                                Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
                                                IMAGE_SIZE::TABS_ICON_IN_MENU_48,
                                                this};
  mTagsMenu->InitDefaultActionFromQSetting(JsonOpMemoryKey::TAGS_OPERATION(), true);

  mUpdateFieldTb = new (std::nothrow) QToolBar{"Duration/Size/Hash field update", this};
  CHECK_NULLPTR_RETURN_VOID(mUpdateFieldTb);
  mUpdateFieldTb->addAction(inst._UPDATE_DURATION_FIELD);
  mUpdateFieldTb->addAction(inst._UPDATE_SIZE_FIELD);
  mUpdateFieldTb->addAction(inst._UPDATE_HASH_FIELD);
  mUpdateFieldTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mUpdateFieldTb->setOrientation(Qt::Orientation::Vertical);
  mUpdateFieldTb->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  mUpdateFieldTb->setStyleSheet("QToolBar { max-width: 256px; }");
  SetLayoutAlightment(mUpdateFieldTb->layout(), Qt::AlignmentFlag::AlignLeft);

  addAction(g_viewActions()._JSON_VIEW);
  addSeparator();
  addAction(inst._SAVE_CURRENT_CHANGES);
  addSeparator();
  addAction(inst._RENAME_JSON_AND_RELATED_FILES);
  addSeparator();
  addWidget(mSyncCacheFileSystemTb);
  addSeparator();
  addWidget(mFieldCaseOperTb);
  addSeparator();
  addWidget(mInitFormatStudioCastField);
  addWidget(mInferStudioCastFromSelection);
  addSeparator();
  addWidget(mStudioMenu);
  addWidget(mCastMenu);
  addWidget(mTagsMenu);
  addSeparator();
  addWidget(mUpdateFieldTb);

  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
}
