#include "RibbonJson.h"
#include "public/PublicMacro.h"
#include "public/PublicVariable.h"
#include "Actions/JsonActions.h"
#include "Actions/ViewActions.h"

RibbonJson::RibbonJson(const QString& title, QWidget* parent)  //
    : QToolBar{title, parent}                                  //
{
  auto& inst = g_JsonActions();

  syncCacheFileSystemTB = new (std::nothrow) QToolBar{"Sync cache/file system", this};
  CHECK_NULLPTR_RETURN_VOID(syncCacheFileSystemTB);
  syncCacheFileSystemTB->addAction(inst._SYNC_NAME_FIELD_BY_FILENAME);
  syncCacheFileSystemTB->addAction(inst._RELOAD_JSON_FROM_FROM_DISK);
  syncCacheFileSystemTB->addAction(inst._EXPORT_CAST_STUDIO_TO_DICTION);
  syncCacheFileSystemTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  syncCacheFileSystemTB->setOrientation(Qt::Orientation::Vertical);
  syncCacheFileSystemTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  syncCacheFileSystemTB->setStyleSheet("QToolBar { max-width: 256px; }");

  caseControlTB = new (std::nothrow) QToolBar{"Case Control", this};
  CHECK_NULLPTR_RETURN_VOID(caseControlTB);
  caseControlTB->addAction(inst._CAPITALIZE_FIRST_LETTER_OF_EACH_WORD);
  caseControlTB->addSeparator();
  caseControlTB->addAction(inst._LOWER_ALL_WORDS);
  caseControlTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  caseControlTB->setOrientation(Qt::Orientation::Vertical);
  caseControlTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  caseControlTB->setStyleSheet("QToolBar { max-width: 256px; }");

  studioCastTagsFieldfOperationTB = new (std::nothrow) QToolBar{"Studio/Cast/Tags Field Operation", this};
  CHECK_NULLPTR_RETURN_VOID(studioCastTagsFieldfOperationTB);
  constructFieldsTB = new (std::nothrow) QToolBar{"Clear fields Operation", studioCastTagsFieldfOperationTB};
  CHECK_NULLPTR_RETURN_VOID(constructFieldsTB);
  constructFieldsTB->addAction(inst._CLEAR_CAST);
  constructFieldsTB->addAction(inst._CLEAR_STUDIO);
  constructFieldsTB->addAction(inst._CLEAR_TAGS);
  constructFieldsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  constructFieldsTB->setOrientation(Qt::Orientation::Vertical);
  constructFieldsTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  constructFieldsTB->setStyleSheet("QToolBar { max-width: 256px; }");

  hintFieldsTB = new (std::nothrow) QToolBar{"Hint and Format", studioCastTagsFieldfOperationTB};
  CHECK_NULLPTR_RETURN_VOID(hintFieldsTB);
  hintFieldsTB->addAction(inst._AI_HINT_CAST_STUDIO);
  hintFieldsTB->addSeparator();
  hintFieldsTB->addAction(inst._FORMATTER);
  hintFieldsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  hintFieldsTB->setOrientation(Qt::Orientation::Vertical);
  hintFieldsTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  hintFieldsTB->setStyleSheet("QToolBar { max-width: 256px; }");

  studioTB = new (std::nothrow) QToolBar("Studio Edit Toolbar", studioCastTagsFieldfOperationTB);
  CHECK_NULLPTR_RETURN_VOID(studioTB);
  studioTB->addAction(inst._INIT_STUDIO_CAST);
  studioTB->addAction(inst._STUDIO_FIELD_SET);
  studioTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  studioTB->setOrientation(Qt::Orientation::Vertical);
  studioTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  studioTB->setStyleSheet("QToolBar { max-width: 256px; }");

  castEditTB = new (std::nothrow) QToolBar("Cast Edit Toolbar", studioCastTagsFieldfOperationTB);
  CHECK_NULLPTR_RETURN_VOID(castEditTB);
  castEditTB->addAction(inst._CAST_FIELD_SET);
  castEditTB->addAction(inst._CAST_FIELD_APPEND);
  castEditTB->addAction(inst._CAST_FIELD_RMV);
  castEditTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  castEditTB->setOrientation(Qt::Orientation::Vertical);
  castEditTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  castEditTB->setStyleSheet("QToolBar { max-width: 256px; }");

  tagsEditTB = new (std::nothrow) QToolBar("Tags Edit Toolbar", studioCastTagsFieldfOperationTB);
  CHECK_NULLPTR_RETURN_VOID(tagsEditTB);
  tagsEditTB->addAction(inst._TAGS_FIELD_SET);
  tagsEditTB->addAction(inst._TAGS_FIELD_APPEND);
  tagsEditTB->addAction(inst._TAGS_FIELD_RMV);
  tagsEditTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  tagsEditTB->setOrientation(Qt::Orientation::Vertical);
  tagsEditTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  tagsEditTB->setStyleSheet("QToolBar { max-width: 256px; }");

  castFromSentenceTb = new (std::nothrow) QToolBar("Cast from sentence Toolbar", studioCastTagsFieldfOperationTB);
  CHECK_NULLPTR_RETURN_VOID(castFromSentenceTb);
  castFromSentenceTb->addAction(inst._ADD_SELECTED_CAST_SENTENCE);
  castFromSentenceTb->addSeparator();
  castFromSentenceTb->addAction(inst._EXTRACT_UPPERCASE_CAST);
  castFromSentenceTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  castFromSentenceTb->setOrientation(Qt::Orientation::Vertical);
  castFromSentenceTb->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  castFromSentenceTb->setStyleSheet("QToolBar { max-width: 256px; }");

  studioCastTagsFieldfOperationTB->addWidget(constructFieldsTB);
  studioCastTagsFieldfOperationTB->addSeparator();
  studioCastTagsFieldfOperationTB->addWidget(hintFieldsTB);
  studioCastTagsFieldfOperationTB->addSeparator();
  studioCastTagsFieldfOperationTB->addWidget(studioTB);
  studioCastTagsFieldfOperationTB->addWidget(castEditTB);
  studioCastTagsFieldfOperationTB->addWidget(tagsEditTB);
  studioCastTagsFieldfOperationTB->addWidget(castFromSentenceTb);

  addAction(g_viewActions()._JSON_VIEW);
  addSeparator();
  addAction(inst._SAVE_CURRENT_CHANGES);
  addSeparator();
  addAction(inst._RENAME_JSON_AND_RELATED_FILES);
  addSeparator();
  addWidget(syncCacheFileSystemTB);
  addSeparator();
  addWidget(caseControlTB);
  addSeparator();
  addWidget(studioCastTagsFieldfOperationTB);

  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
}
