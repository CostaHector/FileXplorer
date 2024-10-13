#include "JsonEditorActions.h"
#include <QApplication>
#include <QMenu>
#include <QStyle>

JsonEditorActions::JsonEditorActions(QObject* parent)
    : QObject{parent},
      _STUDIO_INFORMATION(new QAction(QIcon(":img/STATS"), tr("Studios statistics"), this)),
      _EDIT_STUDIOS(new QAction(QIcon(":img/PRODUCTION_STUDIOS"), tr("Edit Studios map"), this)),
      _RELOAD_STUDIOS(new QAction(QIcon(":img/RELOAD_FROM_DISK"), tr("Reload Studios"), this)),
      _STUDIO_ACTIONS{new QActionGroup(this)},

      _PERFORMERS_INFORMATION(new QAction(QIcon(":img/STATS"), tr("Performers statistics"), this)),
      _EDIT_PERFS(new QAction(QIcon(":img/PERFORMERS"), tr("Edit Performers list"), this)),
      _RELOAD_PERFS(new QAction(QIcon(":img/RELOAD_FROM_DISK"), tr("Reload Performers"), this)),
      _PERFS_ACTIONS{new QActionGroup(this)},

      _AKA_PERFORMERS_INFORMATION(new QAction(QIcon(":img/STATS"), tr("Aka Performers statistics"), this)),
      _EDIT_PERF_AKA(new QAction(QIcon(":img/EDIT_AKA_FILE"), tr("Edit AKA perf file"), this)),
      _RELOAD_PERF_AKA(new QAction(QIcon(":img/RELOAD_FROM_DISK"), tr("Reload AKA perf"), this)),
      _PERFS_AKA_ACTIONS{new QActionGroup(this)},

      _BROWSE_AND_SELECT_THE_FOLDER(new QAction(QIcon(":img/LOAD_A_PATH"), tr("Browse"), this)),
      _CLR_TO_BE_EDITED_LIST(new QAction(QIcon(":img/EMPTY_LISTWIDGET"), tr("Clear list"), this)),
      _FILE_LOAD_ACTIONS{new QActionGroup(this)},

      _LAST_FILE(new QAction(QIcon(":img/LAST_JSON"), tr("Last"), this)),
      _NEXT_FILE(new QAction(QIcon(":img/NEXT_JSON"), tr("Next"), this)),
      _DONE_AND_NEXT(new QAction(QIcon(":img/DONE_AND_NEXT"), tr("Done and next"), this)),
      _AUTO_SKIP(new QAction(QIcon(":img/AUTO_SKIP"), tr("Autoskip"), this)),
      _COMPLETE_PERFS_COUNT(new QAction(QIcon(":img/COMPLETE_PERFS_COUNT"), tr("Perf Cnt"), this)),
      _QUICK_EDIT_ACTIONS{new QActionGroup(this)},

      _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD(new QAction(QIcon(":img/CAPITALIZE_EACH_WORD"), tr("Capitalize"), this)),
      _LOWER_ALL_WORDS(new QAction(QIcon(":img/RENAME_LOWER_CASE"), tr("lowercase"), this)),
      _FORMATTER(new QAction(QIcon(":img/FORMAT"), tr("Formatter"), this)),
      _RELOAD_JSON_FROM_FROM_DISK(new QAction(QIcon(":img/RELOAD_FROM_DISK"), tr("Reload now"), this)),
      _ADD_SELECTED_PERFORMER(new QAction(QIcon(":img/NEW_FILE_FOLDER_PATH"), tr("Append to perfs"), this)),
      _EXTRACT_CAPITALIZED_PERFORMER(new QAction(QIcon(":img/NEW_FILE_FOLDER_PATH"), tr("Append capitalized to perfs"), this)),
      _TEXT_EDIT_ACTIONS{new QActionGroup(this)},

      _SAVE(new QAction(QIcon(":img/SAVED"), tr("save"), this)),
      _CANCEL(new QAction(QIcon(":img/NOT_SAVED"), tr("cancel"), this)),
      _SUBMIT(new QAction(QIcon(":img/APPROVAL"), tr("submit"), this)),
      _FILE_SAVE_ACTIONS{new QActionGroup(this)},

      _AI_HINT(new QAction(QIcon(":img/AI_IDEA"), tr("AI Hint"), this)),
      _LEARN_PERFORMERS_FROM_JSON(new QAction(QIcon(":img/AI_LEARN"), tr("AI Library"), this)),
      _AI_ACTIONS{new QActionGroup(this)},

      _SELECT_CURRENT_FOLDER{new QAction(QIcon(":img/LOAD_JSONS_FROM_CURRENT_PATH"), tr("Select Current path"))},
      _CONSTRUCT_JSONS_FOR_VIDS{new QAction(tr("Construct jsons by file properties"))},
      _APPEND_PERFORMERS{new QAction(QIcon(":img/APPEND_PERFORMERS"), tr("Append performers"))},
      _SET_STUDIO{new QAction(QIcon(":img/PRODUCTION_STUDIOS"), tr("Set production studio"))},
      _CONSTRUCT_PERFORMER_KVP{new QAction(QIcon(":img/PERFORMERS"), tr("Construct performers k-v pair"))},
      _CONSTRUCT_STUDIO_KVP{new QAction(QIcon(":img/PRODUCTION_STUDIOS"), tr("Construct studio k-v pair"))},
      _CLR_PERFORMERS_STUDIO_VALUE{new QAction(tr("Clear performers and studio values"))},
      _BATCH_EDIT_TOOL_ACTIONS{new QActionGroup(this)} {
  _FORMATTER->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_I));
  _FORMATTER->setShortcutVisibleInContextMenu(true);
  _FORMATTER->setToolTip(QString("<b>%1 (%2)</b><br/> Format current json(not stage). e.g., A,B -> A, B.")
                             .arg(_FORMATTER->text())
                             .arg(_FORMATTER->shortcut().toString()));

  _SUBMIT->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_S));
  _SUBMIT->setShortcutVisibleInContextMenu(true);
  _SUBMIT->setToolTip(
      QString("<b>%1 (%2)</b><br/> (CANNOT RECOVER!) Submit all staged changes").arg(_SUBMIT->text()).arg(_SUBMIT->shortcut().toString()));

  _ADD_SELECTED_PERFORMER->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_D));
  _ADD_SELECTED_PERFORMER->setShortcutVisibleInContextMenu(true);
  _ADD_SELECTED_PERFORMER->setToolTip(QString("<b>%1 (%2)</b><br/> Append selection to performers lineeditor")
                                          .arg(_ADD_SELECTED_PERFORMER->text())
                                          .arg(_ADD_SELECTED_PERFORMER->shortcut().toString()));

  _EXTRACT_CAPITALIZED_PERFORMER->setToolTip(QString("<b>%1 (%2)</b><br/> Append capitalized name in selection to performers lineeditor")
                                                 .arg(_EXTRACT_CAPITALIZED_PERFORMER->text())
                                                 .arg(_EXTRACT_CAPITALIZED_PERFORMER->shortcut().toString()));

  _SAVE->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_S));
  _SAVE->setShortcutVisibleInContextMenu(true);
  _SAVE->setToolTip(QString("<b>%1 (%2)</b><br/> Stage current changes").arg(_SAVE->text()).arg(_SAVE->shortcut().toString()));

  _CANCEL->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_R));
  _CANCEL->setShortcutVisibleInContextMenu(true);
  _CANCEL->setToolTip(QString("<b>%1 (%2)</b><br/> Cancel current changes").arg(_CANCEL->text()).arg(_CANCEL->shortcut().toString()));

  _BROWSE_AND_SELECT_THE_FOLDER->setShortcut(
      QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_O));
  _BROWSE_AND_SELECT_THE_FOLDER->setShortcutVisibleInContextMenu(true);
  _BROWSE_AND_SELECT_THE_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/> Browse and select the folder. Then load jsons file from it")
                                                .arg(_BROWSE_AND_SELECT_THE_FOLDER->text())
                                                .arg(_BROWSE_AND_SELECT_THE_FOLDER->shortcut().toString()));
  _CLR_TO_BE_EDITED_LIST->setToolTip(
      QString("<b>%1 (%2)</b><br/> Clear to-be-edit list").arg(_CLR_TO_BE_EDITED_LIST->text()).arg(_CLR_TO_BE_EDITED_LIST->shortcut().toString()));

  _RELOAD_JSON_FROM_FROM_DISK->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_F5));
  _RELOAD_JSON_FROM_FROM_DISK->setShortcutVisibleInContextMenu(true);
  _RELOAD_JSON_FROM_FROM_DISK->setToolTip(QString("<b>%1 (%2)</b><br/> Reload json file from disk")
                                              .arg(_RELOAD_JSON_FROM_FROM_DISK->text())
                                              .arg(_RELOAD_JSON_FROM_FROM_DISK->shortcut().toString()));

  _LAST_FILE->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_J));
  _LAST_FILE->setShortcutVisibleInContextMenu(true);
  _LAST_FILE->setToolTip(QString("<b>%1 (%2)</b><br/> Last one json(if exists)").arg(_LAST_FILE->text()).arg(_LAST_FILE->shortcut().toString()));

  _NEXT_FILE->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_K));
  _NEXT_FILE->setShortcutVisibleInContextMenu(true);
  _NEXT_FILE->setToolTip(QString("<b>%1 (%2)</b><br/> Next one json(if exists)").arg(_NEXT_FILE->text()).arg(_NEXT_FILE->shortcut().toString()));

  _DONE_AND_NEXT->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_Return));
  _DONE_AND_NEXT->setShortcutVisibleInContextMenu(true);
  _DONE_AND_NEXT->setToolTip(QString("<b>%1 (%2)</b><br/> Mark item as done and move to next unfinished item")
                                 .arg(_DONE_AND_NEXT->text())
                                 .arg(_DONE_AND_NEXT->shortcut().toString()));

  _AUTO_SKIP->setCheckable(true);
  _AUTO_SKIP->setChecked(true);

  _COMPLETE_PERFS_COUNT->setShortcutVisibleInContextMenu(true);
  _COMPLETE_PERFS_COUNT->setToolTip(QString("<b>%1 (%2)</b><br/> Set json complete performer count")
                                        .arg(_COMPLETE_PERFS_COUNT->text())
                                        .arg(_COMPLETE_PERFS_COUNT->shortcut().toString()));

  _QUICK_EDIT_ACTIONS->addAction(_LAST_FILE);
  _QUICK_EDIT_ACTIONS->addAction(_NEXT_FILE);
  _QUICK_EDIT_ACTIONS->addAction(_DONE_AND_NEXT);
  _QUICK_EDIT_ACTIONS->addAction(_AUTO_SKIP);
  _QUICK_EDIT_ACTIONS->addAction(_COMPLETE_PERFS_COUNT);
  _QUICK_EDIT_ACTIONS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_U));
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setShortcutVisibleInContextMenu(true);
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setToolTip(QString("<b>%1 (%2)</b><br/> Capitalize first letter of each word in a sentence.")
                                                        .arg(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->text())
                                                        .arg(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->shortcut().toString()));

  _LOWER_ALL_WORDS->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_L));
  _LOWER_ALL_WORDS->setShortcutVisibleInContextMenu(true);
  _LOWER_ALL_WORDS->setToolTip(
      QString("<b>%1 (%2)</b><br/> Lowercase a sentense.").arg(_LOWER_ALL_WORDS->text()).arg(_LOWER_ALL_WORDS->shortcut().toString()));

  _STUDIO_ACTIONS->addAction(_STUDIO_INFORMATION);
  _STUDIO_ACTIONS->addAction(_EDIT_STUDIOS);
  _STUDIO_ACTIONS->addAction(_RELOAD_STUDIOS);

  _PERFS_ACTIONS->addAction(_PERFORMERS_INFORMATION);
  _PERFS_ACTIONS->addAction(_EDIT_PERFS);
  _PERFS_ACTIONS->addAction(_RELOAD_PERFS);

  _PERFS_AKA_ACTIONS->addAction(_AKA_PERFORMERS_INFORMATION);
  _PERFS_AKA_ACTIONS->addAction(_EDIT_PERF_AKA);
  _PERFS_AKA_ACTIONS->addAction(_RELOAD_PERF_AKA);

  _FILE_LOAD_ACTIONS->addAction(_BROWSE_AND_SELECT_THE_FOLDER);
  _FILE_LOAD_ACTIONS->addAction(_CLR_TO_BE_EDITED_LIST);

  _TEXT_EDIT_ACTIONS->addAction(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD);
  _TEXT_EDIT_ACTIONS->addAction(_LOWER_ALL_WORDS);
  _TEXT_EDIT_ACTIONS->addAction(_FORMATTER);
  _TEXT_EDIT_ACTIONS->addAction(_RELOAD_JSON_FROM_FROM_DISK);
  _TEXT_EDIT_ACTIONS->addAction(_ADD_SELECTED_PERFORMER);
  _TEXT_EDIT_ACTIONS->addAction(_EXTRACT_CAPITALIZED_PERFORMER);

  _AI_ACTIONS->addAction(_AI_HINT);
  _AI_ACTIONS->addAction(_LEARN_PERFORMERS_FROM_JSON);

  _FILE_SAVE_ACTIONS->addAction(_SAVE);
  _FILE_SAVE_ACTIONS->addAction(_CANCEL);
  _FILE_SAVE_ACTIONS->addAction(_SUBMIT);
  _FILE_SAVE_ACTIONS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  _AI_HINT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_H));
  _AI_HINT->setToolTip(QString("<b>%1 (%2)</b><br/> Give you performers list hint").arg(_AI_HINT->text()).arg(_AI_HINT->shortcut().toString()));

  _LEARN_PERFORMERS_FROM_JSON->setToolTip(
      QString("<b>%1 (%2)</b><br/> Learn performers from value of key \"Performers\" in json file. \n Improve its performers hint capability.")
          .arg(_LEARN_PERFORMERS_FROM_JSON->text())
          .arg(_LEARN_PERFORMERS_FROM_JSON->shortcut().toString()));

  _SELECT_CURRENT_FOLDER->setCheckable(false);
  _SELECT_CURRENT_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/>Load json files from current view root path.")
                                         .arg(_SELECT_CURRENT_FOLDER->text(), _SELECT_CURRENT_FOLDER->shortcut().toString()));

  _CONSTRUCT_JSONS_FOR_VIDS->setToolTip(QString("<b>%1 (%2)</b><br/>Construct json file for each vids under current path if json not exists.")
                                            .arg(_CONSTRUCT_JSONS_FOR_VIDS->text(), _CONSTRUCT_JSONS_FOR_VIDS->shortcut().toString()));

  _APPEND_PERFORMERS->setToolTip(QString("<b>%1 (%2)</b><br/>Input performers and append them json files under current path")
                                     .arg(_APPEND_PERFORMERS->text(), _APPEND_PERFORMERS->shortcut().toString()));

  _SET_STUDIO->setToolTip(QString("<b>%1 (%2)</b><br/>Input and set studio for json files under current path")
                              .arg(_SET_STUDIO->text(), _SET_STUDIO->shortcut().toString()));

  _CONSTRUCT_PERFORMER_KVP->setToolTip(QString("<b>%1 (%2)</b><br/>Add and construct performers key-value pair for json files under current path."
                                               "For performer value is non-empty, the json file will be skipped.")
                                           .arg(_CONSTRUCT_PERFORMER_KVP->text(), _CONSTRUCT_PERFORMER_KVP->shortcut().toString()));

  _CONSTRUCT_STUDIO_KVP->setToolTip(QString("<b>%1 (%2)</b><br/>Add production studio key-value pair for json files under current path."
                                            "For studio value is non-empty, the json file will be skipped.")
                                        .arg(_CONSTRUCT_STUDIO_KVP->text(), _CONSTRUCT_STUDIO_KVP->shortcut().toString()));

  _CLR_PERFORMERS_STUDIO_VALUE->setToolTip(QString("<b>%1 (%2)</b><br/>Clear both performers and studio values for json files under current path.")
                                               .arg(_CLR_PERFORMERS_STUDIO_VALUE->text(), _CLR_PERFORMERS_STUDIO_VALUE->shortcut().toString()));

  _BATCH_EDIT_TOOL_ACTIONS->addAction(_SELECT_CURRENT_FOLDER);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_CONSTRUCT_JSONS_FOR_VIDS);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_APPEND_PERFORMERS);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_SET_STUDIO);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_CONSTRUCT_PERFORMER_KVP);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_CONSTRUCT_STUDIO_KVP);
  _BATCH_EDIT_TOOL_ACTIONS->addAction(_CLR_PERFORMERS_STUDIO_VALUE);
}

QActionGroup* JsonEditorActions::GetSystemActions() {
  auto* _SYSTEM_ACTIONS = new QActionGroup(this);

  _SYSTEM_ACTIONS->addAction(_RENAME_THIS_FILE);
  _SYSTEM_ACTIONS->addAction(_REVEAL_IN_EXPLORER);
  _SYSTEM_ACTIONS->addAction(_OPEN_THIS_FILE);

  _RENAME_THIS_FILE->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_F2));
  _RENAME_THIS_FILE->setShortcutVisibleInContextMenu(true);

  _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::AltModifier | Qt::Key_R));
  _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%1 (%2)</b><br/> Reveal the json in its parent folder.")
                                      .arg(_REVEAL_IN_EXPLORER->text())
                                      .arg(_REVEAL_IN_EXPLORER->shortcut().toString()));

  _OPEN_THIS_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _OPEN_THIS_FILE->setShortcutVisibleInContextMenu(true);
  _OPEN_THIS_FILE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Open this json file.").arg(_OPEN_THIS_FILE->text()).arg(_OPEN_THIS_FILE->shortcut().toString()));

  return _SYSTEM_ACTIONS;
}

QMenuBar* JsonEditorActions::GetJsonMenuBar(QWidget* parent) {
  auto* m_menuBar = new QMenuBar(parent);
  QMenu* fileMenu = new QMenu("File", m_menuBar);
  fileMenu->addActions(_FILE_LOAD_ACTIONS->actions());
  fileMenu->addSeparator();
  fileMenu->addActions(_QUICK_EDIT_ACTIONS->actions());
  fileMenu->addSeparator();
  fileMenu->addActions(_AI_ACTIONS->actions());
  QMenu* editMenu = new QMenu("Edit", m_menuBar);
  editMenu->addActions(_TEXT_EDIT_ACTIONS->actions());
  editMenu->addSeparator();
  editMenu->addActions(_FILE_SAVE_ACTIONS->actions());
  QMenu* productionStudioMenu = new QMenu("Studio", m_menuBar);
  productionStudioMenu->addActions(_STUDIO_ACTIONS->actions());
  QMenu* performerMenu = new QMenu("Performer", m_menuBar);
  performerMenu->addActions(_PERFS_ACTIONS->actions());
  performerMenu->addSeparator();
  performerMenu->addActions(_PERFS_AKA_ACTIONS->actions());

  m_menuBar->addMenu(fileMenu);
  m_menuBar->addMenu(editMenu);
  m_menuBar->addMenu(productionStudioMenu);
  m_menuBar->addMenu(performerMenu);
  return m_menuBar;
}

QToolBar* JsonEditorActions::GetJsonToolBar(QWidget* parent) {
  auto* m_editorToolBar = new QToolBar("Json editor actions", parent);
  m_editorToolBar->addActions(_FILE_LOAD_ACTIONS->actions());
  m_editorToolBar->addSeparator();
  m_editorToolBar->addActions(_QUICK_EDIT_ACTIONS->actions());
  m_editorToolBar->addSeparator();
  m_editorToolBar->addActions(_TEXT_EDIT_ACTIONS->actions());
  m_editorToolBar->addSeparator();
  m_editorToolBar->addActions(_FILE_SAVE_ACTIONS->actions());
  m_editorToolBar->addSeparator();
  m_editorToolBar->addActions(_AI_ACTIONS->actions());
  m_editorToolBar->addSeparator();
  m_editorToolBar->addAction(g_jsonEditorActions()._RENAME_THIS_FILE);
  m_editorToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return m_editorToolBar;
}

QMenu* JsonEditorActions::GetJsonToBeEdittedListMenu(QWidget* parent) {
  auto* menu = new QMenu("Json to-be-editted list menu", parent);
  menu->addActions(_SYSTEM_ACTIONS->actions());
  return menu;
}

JsonEditorActions& g_jsonEditorActions() {
  static JsonEditorActions ins;
  return ins;
}
