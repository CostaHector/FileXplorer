#include "JsonEditorActions.h"
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

  _FORMATTER = new (std::nothrow) QAction(QIcon(":img/FORMAT_PAINTER"), "Formatter", this);
  _FORMATTER->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_I));
  _FORMATTER->setShortcutVisibleInContextMenu(true);
  _FORMATTER->setToolTip(QString("<b>%1 (%2)</b><br/> Format current json(not stage)."
                                 "e.g., A,B -> A, B.")
                             .arg(_FORMATTER->text())
                             .arg(_FORMATTER->shortcut().toString()));
  _RELOAD_JSON_FROM_FROM_DISK = new (std::nothrow) QAction(QIcon(":img/RELOAD_FROM_DISK"), "Reload", this);
  _RELOAD_JSON_FROM_FROM_DISK->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_F5));
  _RELOAD_JSON_FROM_FROM_DISK->setShortcutVisibleInContextMenu(true);
  _RELOAD_JSON_FROM_FROM_DISK->setToolTip(QString("<b>%1 (%2)</b><br/> Reload json file from disk")  //
                                              .arg(_RELOAD_JSON_FROM_FROM_DISK->text())              //
                                              .arg(_RELOAD_JSON_FROM_FROM_DISK->shortcut().toString()));
  _ADD_SELECTED_PERFORMER = new (std::nothrow) QAction(QIcon(":img/APPEND_PERFORMERS"), "Add Cast", this);
  _ADD_SELECTED_PERFORMER->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_D));
  _ADD_SELECTED_PERFORMER->setShortcutVisibleInContextMenu(true);
  _ADD_SELECTED_PERFORMER->setToolTip(QString("<b>%1 (%2)</b><br/> Append selection to performers lineeditor")  //
                                          .arg(_ADD_SELECTED_PERFORMER->text())                                 //
                                          .arg(_ADD_SELECTED_PERFORMER->shortcut().toString()));
  _EXTRACT_CAPITALIZED_PERFORMER = new (std::nothrow) QAction("Add Cast\nfrom Capitalized sentence", this);
  _EXTRACT_CAPITALIZED_PERFORMER->setToolTip(QString("<b>%1 (%2)</b><br/> Append capitalized name in selection to Cast lineeditor")  //
                                                 .arg(_EXTRACT_CAPITALIZED_PERFORMER->text())                                        //
                                                 .arg(_EXTRACT_CAPITALIZED_PERFORMER->shortcut().toString()));
  _TEXT_EDIT_ACTIONS = new (std::nothrow) QActionGroup(this);

  _SAVE_CURRENT_CHANGES = new (std::nothrow) QAction{QIcon(":img/SAVE_JSON"), "Save current change", this};
  _SAVE_CURRENT_CHANGES->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_S));
  _SAVE_CURRENT_CHANGES->setShortcutVisibleInContextMenu(true);
  _SAVE_CURRENT_CHANGES->setToolTip(QString("<b>%1 (%2)</b><br/> Stage current changes")  //
                                        .arg(_SAVE_CURRENT_CHANGES->text())               //
                                        .arg(_SAVE_CURRENT_CHANGES->shortcut().toString()));

  _RENAME_THIS_FILE = new (std::nothrow) QAction(QIcon(":img/RENAME"), "Rename this file", this);
  _RENAME_THIS_FILE->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_F2));
  _RENAME_THIS_FILE->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER = new (std::nothrow) QAction(QIcon(":img/REVEAL_IN_EXPLORER"), "Reveal in explorer", this);
  _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::AltModifier | Qt::Key_R));
  _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%1 (%2)</b><br/> Reveal this json in its parent folder.").arg(_REVEAL_IN_EXPLORER->text()).arg(_REVEAL_IN_EXPLORER->shortcut().toString()));
  _OPEN_THIS_FILE = new (std::nothrow) QAction("Open this file", this);
  _OPEN_THIS_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _OPEN_THIS_FILE->setShortcutVisibleInContextMenu(true);
  _OPEN_THIS_FILE->setToolTip(QString("<b>%1 (%2)</b><br/> Open this json file.").arg(_OPEN_THIS_FILE->text()).arg(_OPEN_THIS_FILE->shortcut().toString()));

  _AI_HINT = new (std::nothrow) QAction(QIcon(":img/AI_IDEA"), "Hint", this);
  _AI_HINT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_H));
  _AI_HINT->setToolTip(QString("<b>%1 (%2)</b><br/> Give you cast hint")  //
                           .arg(_AI_HINT->text())
                           .arg(_AI_HINT->shortcut().toString()));
  _LEARN_PERFORMERS_FROM_JSON = new (std::nothrow) QAction(QIcon(":img/AI_LEARN"), "Learn...", this);
  _LEARN_PERFORMERS_FROM_JSON->setToolTip(QString("<b>%1 (%2)</b><br/> Learn cast from value of key \"Cast\" in json file. \n Improve its performers hint capability.")
                                              .arg(_LEARN_PERFORMERS_FROM_JSON->text())
                                              .arg(_LEARN_PERFORMERS_FROM_JSON->shortcut().toString()));

  _SYNC_JSON_NAME_VALUE_BY_FILENAME = new (std::nothrow) QAction(QIcon(":img/SYNC_MODIFICATION_SWITCH"), "Sync Json Name Value");
  _SYNC_JSON_NAME_VALUE_BY_FILENAME->setToolTip(QString("<b>%1 (%2)</b><br/>Sync json name value by corresponding Json file name.")  //
                                                    .arg(_SYNC_JSON_NAME_VALUE_BY_FILENAME->text(), _SYNC_JSON_NAME_VALUE_BY_FILENAME->shortcut().toString()));
  _APPEND_PERFORMERS = new (std::nothrow) QAction(QIcon(":img/APPEND_PERFORMERS"), "Append Cast");
  _APPEND_PERFORMERS->setToolTip(QString("<b>%1 (%2)</b><br/>Input performers and append them json files under current path")  //
                                     .arg(_APPEND_PERFORMERS->text(), _APPEND_PERFORMERS->shortcut().toString()));
  _SET_STUDIO = new (std::nothrow) QAction(QIcon(":img/PRODUCTION_STUDIOS_LIST_FILE"), "Set Studio");
  _SET_STUDIO->setToolTip(QString("<b>%1 (%2)</b><br/>Input and set studio for json files under current path")  //
                              .arg(_SET_STUDIO->text(), _SET_STUDIO->shortcut().toString()));
  _CONSTRUCT_STUDIO_CAST = new (std::nothrow) QAction(QIcon(":img/PERFORMERS_LIST_FILE"), "Construct Studio/Cast");
  _CONSTRUCT_STUDIO_CAST->setToolTip(QString("<b>%1 (%2)</b><br/>Construct and StudioCast key-value pair for json files under current path."
                                             "If studio/cast value is non-empty, skip it.")
                                         .arg(_CONSTRUCT_STUDIO_CAST->text(), _CONSTRUCT_STUDIO_CAST->shortcut().toString()));
  _CLR_PERFORMERS_STUDIO_VALUE = new (std::nothrow) QAction("Clear Studio/Cast values");
  _CLR_PERFORMERS_STUDIO_VALUE->setToolTip(QString("<b>%1 (%2)</b><br/>Clear both performers and studio values for json files under current path.")
                                               .arg(_CLR_PERFORMERS_STUDIO_VALUE->text(), _CLR_PERFORMERS_STUDIO_VALUE->shortcut().toString()));
  _STANDARDLIZE_JSON_KEY = new (std::nothrow) QAction("Standardlize Json Key");
  _BATCH_EDIT_TOOL_ACTIONS = new (std::nothrow) QActionGroup(this);

  _TEXT_EDIT_ACTIONS->addAction(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD);
  _TEXT_EDIT_ACTIONS->addAction(_LOWER_ALL_WORDS);
  _TEXT_EDIT_ACTIONS->addAction(_FORMATTER);
  _TEXT_EDIT_ACTIONS->addAction(_RELOAD_JSON_FROM_FROM_DISK);
  _TEXT_EDIT_ACTIONS->addAction(_ADD_SELECTED_PERFORMER);
  _TEXT_EDIT_ACTIONS->addAction(_EXTRACT_CAPITALIZED_PERFORMER);

  _BATCH_EDIT_TOOL_ACTIONS->addAction(_SELECT_CURRENT_FOLDER);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_SYNC_JSON_NAME_VALUE_BY_FILENAME);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_APPEND_PERFORMERS);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_SET_STUDIO);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_CONSTRUCT_STUDIO_CAST);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_CLR_PERFORMERS_STUDIO_VALUE);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_STANDARDLIZE_JSON_KEY);
}

QToolBar* JsonEditorActions::GetJsonToolBar(QWidget* parent) {
  auto* m_editorToolBar = new (std::nothrow) QToolBar{"Json editor actions", parent};
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
  m_editorToolBar->addAction(_AI_HINT);
  m_editorToolBar->addAction(_LEARN_PERFORMERS_FROM_JSON);
  m_editorToolBar->addSeparator();
  m_editorToolBar->addAction(g_jsonEditorActions()._RENAME_THIS_FILE);
  m_editorToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return m_editorToolBar;
}

QMenu* JsonEditorActions::GetJsonToBeEdittedListMenu(QWidget* parent) {
  auto* menu = new (std::nothrow) QMenu{"Json List Menu", parent};
  menu->addAction(_RENAME_THIS_FILE);
  menu->addAction(_REVEAL_IN_EXPLORER);
  menu->addAction(_OPEN_THIS_FILE);
  return menu;
}

JsonEditorActions& g_jsonEditorActions() {
  static JsonEditorActions ins;
  return ins;
}
