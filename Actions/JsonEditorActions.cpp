#include "JsonEditorActions.h"
#include "public/PublicMacro.h"
#include "public/PublicVariable.h"
#include <QApplication>
#include <QMenu>
#include <QStyle>

JsonEditorActions::JsonEditorActions(QObject* parent)  //
    : QObject{parent}                                  //
{
  _SELECT_CURRENT_FOLDER = new (std::nothrow) QAction{QIcon(":img/LOAD_A_PATH"), "Read Current path"};
  _SELECT_CURRENT_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/>Load json files from current view root path.")  //
                                         .arg(_SELECT_CURRENT_FOLDER->text(),                                    //
                                              _SELECT_CURRENT_FOLDER->shortcut().toString()));
  _CLR_JSON_FILE_LIST = new (std::nothrow) QAction(QIcon(":img/EMPTY_LISTWIDGET"), "Clear list", this);
  _CLR_JSON_FILE_LIST->setToolTip(QString("<b>%1 (%2)</b><br/> Clear jsons files list")  //
                                      .arg(_CLR_JSON_FILE_LIST->text())                  //
                                      .arg(_CLR_JSON_FILE_LIST->shortcut().toString()));

  _LAST_FILE = new (std::nothrow) QAction(QIcon{":img/JSON_FILE_PRECIOUS"}, "Last", this);
  _LAST_FILE->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_BracketLeft));
  _LAST_FILE->setShortcutVisibleInContextMenu(true);
  _LAST_FILE->setToolTip(QString("<b>%1 (%2)</b><br/> Last one json(if exists)")  //
                             .arg(_LAST_FILE->text())                             //
                             .arg(_LAST_FILE->shortcut().toString()));
  _NEXT_FILE = new (std::nothrow) QAction(QIcon{":img/JSON_FILE_NEXT"}, "Next", this);
  _NEXT_FILE->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_BracketRight));
  _NEXT_FILE->setShortcutVisibleInContextMenu(true);
  _NEXT_FILE->setToolTip(QString("<b>%1 (%2)</b><br/> Next one json(if exists)")  //
                             .arg(_NEXT_FILE->text())                             //
                             .arg(_NEXT_FILE->shortcut().toString()));
  _DONE_AND_PREVIOUS = new (std::nothrow) QAction(QIcon{":img/JSON_FILE_NEXT_PREVIOUS"}, "Done & Last", this);
  _DONE_AND_PREVIOUS->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_Return));
  _DONE_AND_PREVIOUS->setShortcutVisibleInContextMenu(true);
  _DONE_AND_PREVIOUS->setToolTip(QString("<b>%1 (%2)</b><br/> Mark item as done and selected to previous unfinished item")  //
                                     .arg(_DONE_AND_PREVIOUS->text())                                                       //
                                     .arg(_DONE_AND_PREVIOUS->shortcut().toString()));
  _DONE_AND_NEXT = new (std::nothrow) QAction(QIcon{":img/JSON_FILE_NEXT_UNFINISHED"}, "Done & Next", this);
  _DONE_AND_NEXT->setShortcut(QKeySequence(Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_Return));
  _DONE_AND_NEXT->setShortcutVisibleInContextMenu(true);
  _DONE_AND_NEXT->setToolTip(QString("<b>%1 (%2)</b><br/> Mark item as done and selected to next unfinished item")  //
                                 .arg(_DONE_AND_NEXT->text())                                                       //
                                 .arg(_DONE_AND_NEXT->shortcut().toString()));
  _AUTO_SKIP = new (std::nothrow) QAction(QIcon(":img/AUTO_SKIP"), "Auto skip", this);
  _AUTO_SKIP->setCheckable(true);
  _AUTO_SKIP->setChecked(true);
  _AUTO_SKIP->setShortcutVisibleInContextMenu(true);
  _AUTO_SKIP->setToolTip(QString("<b>%1 (%2)</b><br/> When click next, it will skip cast count greater than 2")  //
                             .arg(_AUTO_SKIP->text())                                                            //
                             .arg(_AUTO_SKIP->shortcut().toString()));
  _SKIP_IF_CAST_CNT_GT = new (std::nothrow) QAction(QIcon{":img/SKIP_IF"}, "Cast count GT", this);
  _SKIP_IF_CAST_CNT_GT->setShortcutVisibleInContextMenu(true);
  _SKIP_IF_CAST_CNT_GT->setToolTip(QString("<b>%1 (%2)</b><br/> Set json complete performer count")  //
                                       .arg(_SKIP_IF_CAST_CNT_GT->text())                            //
                                       .arg(_SKIP_IF_CAST_CNT_GT->shortcut().toString()));

  // **Submit**
  _SAVE_CURRENT_CHANGES = new (std::nothrow) QAction{QIcon(":img/SUBMIT"), "Submit", this};
  _SAVE_CURRENT_CHANGES->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_S));
  _SAVE_CURRENT_CHANGES->setShortcutVisibleInContextMenu(true);
  _SAVE_CURRENT_CHANGES->setToolTip(QString("<b>%1 (%2)</b><br/> Commit selected row changes")  //
                                        .arg(_SAVE_CURRENT_CHANGES->text())                     //
                                        .arg(_SAVE_CURRENT_CHANGES->shortcut().toString()));

  // **Sync cache/file system**
  _SYNC_NAME_FIELD_BY_FILENAME = new (std::nothrow) QAction(QIcon(":img/SYNC_MODIFICATION_SWITCH"), "Sync Json Name Value");
  _SYNC_NAME_FIELD_BY_FILENAME->setToolTip(QString("<b>%1 (%2)</b><br/>Sync Name Field from json file base name.")  //
                                               .arg(_SYNC_NAME_FIELD_BY_FILENAME->text(), _SYNC_NAME_FIELD_BY_FILENAME->shortcut().toString()));

  _RELOAD_JSON_FROM_FROM_DISK = new (std::nothrow) QAction(QIcon(":img/RELOAD_FROM_DISK"), "Reload from disk", this);
  _RELOAD_JSON_FROM_FROM_DISK->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_F5));
  _RELOAD_JSON_FROM_FROM_DISK->setShortcutVisibleInContextMenu(true);
  _RELOAD_JSON_FROM_FROM_DISK->setToolTip(QString("<b>%1 (%2)</b><br/> Reload json file from disk")  //
                                              .arg(_RELOAD_JSON_FROM_FROM_DISK->text())              //
                                              .arg(_RELOAD_JSON_FROM_FROM_DISK->shortcut().toString()));

  _EXPORT_CAST_STUDIO_TO_DICTION = new (std::nothrow) QAction(QIcon(":img/AI_LEARN"), "Export to Dictionary", this);
  _EXPORT_CAST_STUDIO_TO_DICTION->setToolTip(QString("<b>%1 (%2)</b><br/> Export studio/cast from field `Cast` and `Studio` in json file. \n Improve its cast/studio hint capability.")
                                                 .arg(_EXPORT_CAST_STUDIO_TO_DICTION->text())
                                                 .arg(_EXPORT_CAST_STUDIO_TO_DICTION->shortcut().toString()));

  // **Case Control**
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD = new (std::nothrow) QAction(QIcon{":img/CASE_TITLE"}, "Title", this);
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_U));
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setShortcutVisibleInContextMenu(true);
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setToolTip(QString("<b>%1 (%2)</b><br/> Capitalize first letter of each word in a sentence.")
                                                        .arg(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->text())
                                                        .arg(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->shortcut().toString()));

  _LOWER_ALL_WORDS = new (std::nothrow) QAction(QIcon(":img/RENAME_LOWER_CASE"), "Lower", this);
  _LOWER_ALL_WORDS->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_L));
  _LOWER_ALL_WORDS->setShortcutVisibleInContextMenu(true);
  _LOWER_ALL_WORDS->setToolTip(QString("<b>%1 (%2)</b><br/> Lowercase a sentence.")  //
                                   .arg(_LOWER_ALL_WORDS->text())                    //
                                   .arg(_LOWER_ALL_WORDS->shortcut().toString()));

  // **File Operation**
  _OPEN_THIS_FILE = new (std::nothrow) QAction("Open this file", this);
  _OPEN_THIS_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _OPEN_THIS_FILE->setShortcutVisibleInContextMenu(true);
  _OPEN_THIS_FILE->setToolTip(QString("<b>%1 (%2)</b><br/> Open this json file in system default app.").arg(_OPEN_THIS_FILE->text()).arg(_OPEN_THIS_FILE->shortcut().toString()));
  _REVEAL_IN_EXPLORER = new (std::nothrow) QAction(QIcon(":img/REVEAL_IN_EXPLORER"), "Reveal in explorer", this);
  _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::AltModifier | Qt::Key_R));
  _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%1 (%2)</b><br/> Reveal this json in its parent folder.").arg(_REVEAL_IN_EXPLORER->text()).arg(_REVEAL_IN_EXPLORER->shortcut().toString()));
  _RENAME_JSON_AND_RELATED_FILES = new (std::nothrow) QAction(QIcon(":img/RENAME"), "Rename json\n(also related files)", this);
  _RENAME_JSON_AND_RELATED_FILES->setShortcut(QKeySequence(Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_F2));
  _RENAME_JSON_AND_RELATED_FILES->setShortcutVisibleInContextMenu(true);
  _RENAME_JSON_AND_RELATED_FILES->setToolTip(QString("<b>%1 (%2)</b><br/> Rename this json file and its related file(s)")  //
                                                 .arg(_RENAME_JSON_AND_RELATED_FILES->text())
                                                 .arg(_RENAME_JSON_AND_RELATED_FILES->shortcut().toString()));

  // **Studio/Cast/Tags Field Operation**
  _CLEAR_CAST = new (std::nothrow) QAction(QIcon(":img/CAST_CLEAR"), "Clear Cast");
  _CLEAR_CAST->setToolTip(QString("<b>%1 (%2)</b><br/>Clear Cast in selected rows.").arg(_CLEAR_CAST->text(), _CLEAR_CAST->shortcut().toString()));
  _CLEAR_STUDIO = new (std::nothrow) QAction(QIcon(":img/STUDIO_CLEAR"), "Clear Studio");
  _CLEAR_STUDIO->setToolTip(QString("<b>%1 (%2)</b><br/>Clear Studio in selected rows.").arg(_CLEAR_STUDIO->text(), _CLEAR_STUDIO->shortcut().toString()));
  _CLEAR_TAGS = new (std::nothrow) QAction(QIcon(":img/TAGS_CLEAR"), "Clear Tags");
  _CLEAR_TAGS->setToolTip(QString("<b>%1 (%2)</b><br/>Clear Tags in selected rows.").arg(_CLEAR_TAGS->text(), _CLEAR_TAGS->shortcut().toString()));

  _AI_HINT_CAST_STUDIO = new (std::nothrow) QAction(QIcon(":img/AI_IDEA"), "Cast/Studio Hint", this);
  _AI_HINT_CAST_STUDIO->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_H));
  _AI_HINT_CAST_STUDIO->setToolTip(QString("<b>%1 (%2)</b><br/> Give you cast/studio hint")  //
                                       .arg(_AI_HINT_CAST_STUDIO->text())
                                       .arg(_AI_HINT_CAST_STUDIO->shortcut().toString()));
  _FORMATTER = new (std::nothrow) QAction(QIcon(":img/FORMAT_PAINTER"), "Format Cast/Studio", this);
  _FORMATTER->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_I));
  _FORMATTER->setShortcutVisibleInContextMenu(true);
  _FORMATTER->setToolTip(QString("<b>%1 (%2)</b><br/> Format Cast/Studio Field."
                                 "e.g., A,B -> A, B.")
                             .arg(_FORMATTER->text())
                             .arg(_FORMATTER->shortcut().toString()));

  _INIT_STUDIO_CAST = new (std::nothrow) QAction(QIcon(":img/CAST_SET"), "Init Cast/Studio");
  _INIT_STUDIO_CAST->setToolTip(QString("<b>%1 (%2)</b><br/>Init Cast/Studio Fields if empty.").arg(_INIT_STUDIO_CAST->text(), _INIT_STUDIO_CAST->shortcut().toString()));
  _STUDIO_FIELD_SET = new (std::nothrow) QAction(QIcon(":img/STUDIO"), "Set Studio");
  _STUDIO_FIELD_SET->setToolTip(QString("<b>%1 (%2)</b><br/>Input studio string and used to set Studio field")  //
                                    .arg(_STUDIO_FIELD_SET->text(), _STUDIO_FIELD_SET->shortcut().toString()));
  _CAST_FIELD_SET = new (std::nothrow) QAction(QIcon(":img/CAST_SET"), "Set Cast");
  _CAST_FIELD_SET->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence used to set Cast field")  //
                                  .arg(_CAST_FIELD_SET->text(), _CAST_FIELD_SET->shortcut().toString()));
  _CAST_FIELD_APPEND = new (std::nothrow) QAction(QIcon(":img/CAST_APPEND"), "Add Cast");
  _CAST_FIELD_APPEND->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence append to Cast field")  //
                                     .arg(_CAST_FIELD_APPEND->text(), _CAST_FIELD_APPEND->shortcut().toString()));
  _CAST_FIELD_RMV = new (std::nothrow) QAction(QIcon(":img/CAST_REMOVE"), "Rmv Cast");
  _CAST_FIELD_RMV->setToolTip(QString("<b>%1 (%2)</b><br/>Input a cast used to remove from Cast field")  //
                                  .arg(_CAST_FIELD_RMV->text(), _CAST_FIELD_RMV->shortcut().toString()));
  _TAGS_FIELD_SET = new (std::nothrow) QAction(QIcon(":img/TAGS_SET"), "Set Tags");
  _TAGS_FIELD_SET->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence used to set Tags field")  //
                                  .arg(_TAGS_FIELD_SET->text(), _TAGS_FIELD_SET->shortcut().toString()));
  _TAGS_FIELD_APPEND = new (std::nothrow) QAction(QIcon(":img/TAGS_APPEND"), "Add Tags");
  _TAGS_FIELD_APPEND->setToolTip(QString("<b>%1 (%2)</b><br/>Input a sentence append to Tags field")  //
                                     .arg(_TAGS_FIELD_APPEND->text(), _TAGS_FIELD_APPEND->shortcut().toString()));
  _TAGS_FIELD_RMV = new (std::nothrow) QAction(QIcon(":img/TAGS_REMOVE"), "Rmv Tag");
  _TAGS_FIELD_RMV->setToolTip(QString("<b>%1 (%2)</b><br/>Input a tag used to remove from Tags field")  //
                                  .arg(_TAGS_FIELD_RMV->text(), _TAGS_FIELD_RMV->shortcut().toString()));

  _ADD_SELECTED_CAST_SENTENCE = new (std::nothrow) QAction(QIcon(":img/CAST_APPEND"), "Add Cast", this);
  _ADD_SELECTED_CAST_SENTENCE->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_D));
  _ADD_SELECTED_CAST_SENTENCE->setShortcutVisibleInContextMenu(true);
  _ADD_SELECTED_CAST_SENTENCE->setToolTip(QString("<b>%1 (%2)</b><br/> Append selection to performers lineeditor")  //
                                          .arg(_ADD_SELECTED_CAST_SENTENCE->text())                                 //
                                          .arg(_ADD_SELECTED_CAST_SENTENCE->shortcut().toString()));
  _EXTRACT_UPPERCASE_CAST = new (std::nothrow) QAction("Add Cast\nfrom Capitalized sentence", this);
  _EXTRACT_UPPERCASE_CAST->setToolTip(QString("<b>%1 (%2)</b><br/> Append capitalized name in selection to Cast lineeditor")  //
                                                 .arg(_EXTRACT_UPPERCASE_CAST->text())                                        //
                                                 .arg(_EXTRACT_UPPERCASE_CAST->shortcut().toString()));
  _TEXT_EDIT_ACTIONS = new (std::nothrow) QActionGroup(this);

  _RE_STUDIO_CAST_VALUE = new (std::nothrow) QAction{"Reconstruct Studio/Cast values"};
  _RE_STUDIO_CAST_VALUE->setToolTip(QString("<b>%1 (%2)</b><br/>Reconstruct both Cast and Studio values for json files under current path.")  //
                                        .arg(_RE_STUDIO_CAST_VALUE->text())                                                                   //
                                        .arg(_RE_STUDIO_CAST_VALUE->shortcut().toString()));
  _STANDARDLIZE_JSON_KEY = new (std::nothrow) QAction("Standardlize Json Key");
  _BATCH_EDIT_TOOL_ACTIONS = new (std::nothrow) QActionGroup(this);

  _TEXT_EDIT_ACTIONS->addAction(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD);
  _TEXT_EDIT_ACTIONS->addAction(_LOWER_ALL_WORDS);
  _TEXT_EDIT_ACTIONS->addAction(_FORMATTER);
  _TEXT_EDIT_ACTIONS->addAction(_RELOAD_JSON_FROM_FROM_DISK);
  _TEXT_EDIT_ACTIONS->addAction(_ADD_SELECTED_CAST_SENTENCE);
  _TEXT_EDIT_ACTIONS->addAction(_EXTRACT_UPPERCASE_CAST);

  _BATCH_EDIT_TOOL_ACTIONS->addAction(_SELECT_CURRENT_FOLDER);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_SYNC_NAME_FIELD_BY_FILENAME);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_CAST_FIELD_APPEND);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_STUDIO_FIELD_SET);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_INIT_STUDIO_CAST);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_RE_STUDIO_CAST_VALUE);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_STANDARDLIZE_JSON_KEY);
}

QToolBar* JsonEditorActions::GetJsonToolBar() {
  auto* m_editorToolBar = new (std::nothrow) QToolBar{"Json editor actions"};
  m_editorToolBar->addAction(_SELECT_CURRENT_FOLDER);
  m_editorToolBar->addAction(_CLR_JSON_FILE_LIST);
  m_editorToolBar->addSeparator();
  m_editorToolBar->addAction(_LAST_FILE);
  m_editorToolBar->addAction(_NEXT_FILE);
  m_editorToolBar->addSeparator();
  m_editorToolBar->addAction(_DONE_AND_PREVIOUS);
  m_editorToolBar->addAction(_DONE_AND_NEXT);
  m_editorToolBar->addSeparator();
  m_editorToolBar->addAction(_AUTO_SKIP);
  m_editorToolBar->addAction(_SKIP_IF_CAST_CNT_GT);
  m_editorToolBar->addSeparator();
  m_editorToolBar->addActions(_TEXT_EDIT_ACTIONS->actions());
  m_editorToolBar->addSeparator();
  m_editorToolBar->addAction(_SAVE_CURRENT_CHANGES);
  m_editorToolBar->addSeparator();
  m_editorToolBar->addAction(_AI_HINT_CAST_STUDIO);
  m_editorToolBar->addAction(_EXPORT_CAST_STUDIO_TO_DICTION);
  m_editorToolBar->addSeparator();
  m_editorToolBar->addAction(_RENAME_JSON_AND_RELATED_FILES);
  m_editorToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return m_editorToolBar;
}

QToolBar* JsonEditorActions::GetJsonRibbonToolBar() {
  auto* jsonRibbonTB = new (std::nothrow) QToolBar{"Json Ribbons"};
  CHECK_NULLPTR_RETURN_NULLPTR(jsonRibbonTB);

  auto* syncCacheFileSystemTB = new (std::nothrow) QToolBar{"Sync cache/file system", jsonRibbonTB};
  CHECK_NULLPTR_RETURN_NULLPTR(syncCacheFileSystemTB);
  syncCacheFileSystemTB->addAction(_SYNC_NAME_FIELD_BY_FILENAME);
  syncCacheFileSystemTB->addAction(_RELOAD_JSON_FROM_FROM_DISK);
  syncCacheFileSystemTB->addAction(_EXPORT_CAST_STUDIO_TO_DICTION);
  syncCacheFileSystemTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  syncCacheFileSystemTB->setOrientation(Qt::Orientation::Vertical);
  syncCacheFileSystemTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  syncCacheFileSystemTB->setStyleSheet("QToolBar { max-width: 256px; }");

  auto* caseControlTB = new (std::nothrow) QToolBar{"Case Control", jsonRibbonTB};
  CHECK_NULLPTR_RETURN_NULLPTR(caseControlTB);
  caseControlTB->addAction(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD);
  caseControlTB->addSeparator();
  caseControlTB->addAction(_LOWER_ALL_WORDS);
  caseControlTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  caseControlTB->setOrientation(Qt::Orientation::Vertical);
  caseControlTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  caseControlTB->setStyleSheet("QToolBar { max-width: 256px; }");

  auto* studioCastTagsFieldfOperationTB = new (std::nothrow) QToolBar{"Studio/Cast/Tags Field Operation", jsonRibbonTB};
  CHECK_NULLPTR_RETURN_NULLPTR(studioCastTagsFieldfOperationTB);
  auto* constructFieldsTB = new (std::nothrow) QToolBar{"Clear fields Operation", studioCastTagsFieldfOperationTB};
  CHECK_NULLPTR_RETURN_NULLPTR(constructFieldsTB);
  constructFieldsTB->addAction(_CLEAR_CAST);
  constructFieldsTB->addAction(_CLEAR_STUDIO);
  constructFieldsTB->addAction(_CLEAR_TAGS);
  constructFieldsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  constructFieldsTB->setOrientation(Qt::Orientation::Vertical);
  constructFieldsTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  constructFieldsTB->setStyleSheet("QToolBar { max-width: 256px; }");

  auto* hintFieldsTB = new (std::nothrow) QToolBar{"Hint and Format", studioCastTagsFieldfOperationTB};
  CHECK_NULLPTR_RETURN_NULLPTR(hintFieldsTB);
  hintFieldsTB->addAction(_AI_HINT_CAST_STUDIO);
  hintFieldsTB->addSeparator();
  hintFieldsTB->addAction(_FORMATTER);
  hintFieldsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  hintFieldsTB->setOrientation(Qt::Orientation::Vertical);
  hintFieldsTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  hintFieldsTB->setStyleSheet("QToolBar { max-width: 256px; }");

  auto* studioTB = new (std::nothrow) QToolBar("Studio Edit Toolbar", studioCastTagsFieldfOperationTB);
  CHECK_NULLPTR_RETURN_NULLPTR(studioTB);
  studioTB->addAction(_INIT_STUDIO_CAST);
  studioTB->addAction(_STUDIO_FIELD_SET);
  studioTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  studioTB->setOrientation(Qt::Orientation::Vertical);
  studioTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  studioTB->setStyleSheet("QToolBar { max-width: 256px; }");

  auto* castEditTB = new (std::nothrow) QToolBar("Cast Edit Toolbar", studioCastTagsFieldfOperationTB);
  CHECK_NULLPTR_RETURN_NULLPTR(castEditTB);
  castEditTB->addAction(_CAST_FIELD_SET);
  castEditTB->addAction(_CAST_FIELD_APPEND);
  castEditTB->addAction(_CAST_FIELD_RMV);
  castEditTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  castEditTB->setOrientation(Qt::Orientation::Vertical);
  castEditTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  castEditTB->setStyleSheet("QToolBar { max-width: 256px; }");

  auto* tagsEditTB = new (std::nothrow) QToolBar("Tags Edit Toolbar", studioCastTagsFieldfOperationTB);
  CHECK_NULLPTR_RETURN_NULLPTR(tagsEditTB);
  tagsEditTB->addAction(_TAGS_FIELD_SET);
  tagsEditTB->addAction(_TAGS_FIELD_APPEND);
  tagsEditTB->addAction(_TAGS_FIELD_RMV);
  tagsEditTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  tagsEditTB->setOrientation(Qt::Orientation::Vertical);
  tagsEditTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  tagsEditTB->setStyleSheet("QToolBar { max-width: 256px; }");

  studioCastTagsFieldfOperationTB->addWidget(constructFieldsTB);
  studioCastTagsFieldfOperationTB->addSeparator();
  studioCastTagsFieldfOperationTB->addWidget(hintFieldsTB);
  studioCastTagsFieldfOperationTB->addSeparator();
  studioCastTagsFieldfOperationTB->addWidget(studioTB);
  studioCastTagsFieldfOperationTB->addWidget(castEditTB);
  studioCastTagsFieldfOperationTB->addWidget(tagsEditTB);

  jsonRibbonTB->addAction(_SAVE_CURRENT_CHANGES);
  jsonRibbonTB->addAction(_RENAME_JSON_AND_RELATED_FILES);
  jsonRibbonTB->addSeparator();
  jsonRibbonTB->addWidget(syncCacheFileSystemTB);
  jsonRibbonTB->addSeparator();
  jsonRibbonTB->addWidget(caseControlTB);
  jsonRibbonTB->addSeparator();
  jsonRibbonTB->addWidget(studioCastTagsFieldfOperationTB);
  jsonRibbonTB->addSeparator();
  jsonRibbonTB->addAction(_ADD_SELECTED_CAST_SENTENCE);
  jsonRibbonTB->addAction(_EXTRACT_UPPERCASE_CAST);

  jsonRibbonTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return jsonRibbonTB;
}

QMenu* JsonEditorActions::GetJsonToBeEdittedListMenu(QWidget* parent) {
  auto* menu = new (std::nothrow) QMenu{"Json List Menu", parent};
  menu->addAction(_RENAME_JSON_AND_RELATED_FILES);
  menu->addAction(_REVEAL_IN_EXPLORER);
  menu->addAction(_OPEN_THIS_FILE);
  return menu;
}

JsonEditorActions& g_jsonEditorActions() {
  static JsonEditorActions ins;
  return ins;
}
