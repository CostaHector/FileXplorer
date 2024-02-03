#include "JsonEditorActions.h"
#include <QApplication>
#include <QStyle>

JsonEditorActions::JsonEditorActions(QObject* parent)
    : QObject{parent},
      _STUDIO_INFORMATION(new QAction(tr("Studios info"), this)),
      _EDIT_STUDIOS(new QAction(tr("Edit Studios"), this)),
      _RELOAD_STUDIOS(new QAction(tr("Reload Studios"), this)),

      _PERFORMERS_INFORMATION(new QAction(tr("Performers info"), this)),
      _EDIT_PERFS(new QAction(tr("Edit Performers"), this)),
      _RELOAD_PERFS(new QAction(tr("Reload Performers"), this)),

      _AKA_PERFORMERS_INFORMATION(new QAction(tr("Aka Performers info"), this)),
      _EDIT_PERF_AKA(new QAction(QIcon(":/themes/EDIT_AKA_FILE"), tr("Edit AKA"), this)),
      _RELOAD_PERF_AKA(new QAction(tr("Reload AKA"), this)),

      _SELECT_A_FOLDER_AND_LOAD_JSON(new QAction(QIcon(":/themes/SELECT_A_FOLDER_AND_LOAD_JSON"), tr("Load"), this)),
      _EMPTY_JSONS_LISTWIDGET(new QAction(QIcon(":/themes/EMPTY_LISTWIDGET"), tr("Empty"), this)),
      _LAST_JSON(new QAction(QIcon(":/themes/LAST_JSON"), tr("Last"), this)),
      _NEXT_JSON(new QAction(QIcon(":/themes/NEXT_JSON"), tr("Next"), this)),
      _AUTO_SKIP(new QAction(QIcon(":/themes/AUTO_SKIP"), tr("Autoskip"), this)),
      _CONDITION_NOT(new QAction(QIcon(":/themes/CONDITION_NOT"), tr("WhenNot"), this)),
      FILES_ACTIONS(new QActionGroup(this)),

      _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD(new QAction(QIcon(":/themes/CAPITALIZE_EACH_WORD"), tr("Capitalize"), this)),
      _LOWER_ALL_WORDS(new QAction(QIcon(":/themes/RENAME_LOWER_CASE"), tr("lowercase"), this)),
      _FORMATTER(new QAction(QIcon(":/themes/FORMAT"), tr("Format"), this)),
      _RELOAD_JSON_FROM_FROM_DISK(new QAction(QIcon(":/themes/RELOAD_JSON_FROM_FILE"), tr("From Disk"), this)),
      _ADD_SELECTED_PERFORMER(new QAction(QIcon(":/themes/NEW_FILE_FOLDER_PATH"), tr("Add"), this)),
      _SAVE(new QAction(QIcon(":/themes/SAVED"), tr("save"), this)),
      _CANCEL(new QAction(QIcon(":/themes/NOT_SAVED"), tr("cancel"), this)),
      _SUBMIT(new QAction(QIcon(":/themes/SUBMIT"), tr("submit"), this)),
      EDIT_ACTIONS(new QActionGroup(this)),

      _REVEAL_IN_EXPLORER(new QAction(QIcon(":/themes/REVEAL_IN_EXPLORER"), tr("Reveal"), this)),
      _OPEN_THIS_FILE(new QAction(tr("Open"), this)),
      _HINT(new QAction(QIcon(":/themes/PERFORMERS_LIST_HINT"), tr("Hint"), this)),
      _LEARN_PERFORMERS_FROM_JSON(new QAction(QIcon(":/themes/LEARN_PERFORMERS_FROM_JSON"), tr("+Lib"), this)),

      LOAD_JSONS_FROM_CURRENT_FOLDER{new QAction(QIcon(":/themes/LOAD_JSONS_FROM_CURRENT_PATH"), tr("Load jsons"))},
      CONSTRUCT_JSONS_FOR_VIDS{new QAction(tr("Construct jsons"))},
      JSON_ADD_PERFORMERS{new QAction(tr("Add performers"))},
      JSON_SET_PRODUCTION_STUDIO{new QAction(tr("Set production studio"))},
      JSON_ADD_PERFORMER_KEY_VALUE_PAIR{new QAction(tr("Add performer k-v pair"))},
      JSON_ADD_PRODUCTION_STUDIO_KEY_VALUE_PAIR{new QAction(tr("Add production studio k-v pair"))},
      JSON_CLEAR_PERFORMERS_PROD_STUDIO_NAME{new QAction(tr("Clear performers and prod studio"))},
      BATCH_FILES_ACTIONS(new QActionGroup(this)) {
  _FORMATTER->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_I));
  _FORMATTER->setToolTip(QString("<b>%1 (%2)</b><br/> Format current json(not stage). e.g., A,B -> A, B.")
                             .arg(_FORMATTER->text())
                             .arg(_FORMATTER->shortcut().toString()));

  _NEXT_JSON->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_Period));
  _NEXT_JSON->setToolTip(QString("<b>%1 (%2)</b><br/> Next one json(if exists)").arg(_NEXT_JSON->text()).arg(_NEXT_JSON->shortcut().toString()));

  _LAST_JSON->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_Comma));
  _LAST_JSON->setToolTip(QString("<b>%1 (%2)</b><br/> Last one json(if exists)").arg(_LAST_JSON->text()).arg(_LAST_JSON->shortcut().toString()));

  _SUBMIT->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_S));
  _SUBMIT->setToolTip(
      QString("<b>%1 (%2)</b><br/> (CANNOT RECOVER!) Submit all staged changes").arg(_SUBMIT->text()).arg(_SUBMIT->shortcut().toString()));

  _ADD_SELECTED_PERFORMER->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_D));
  _ADD_SELECTED_PERFORMER->setToolTip(QString("<b>%1 (%2)</b><br/> Add selected text to performer")
                                          .arg(_ADD_SELECTED_PERFORMER->text())
                                          .arg(_ADD_SELECTED_PERFORMER->shortcut().toString()));

  _SAVE->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_S));
  _SAVE->setToolTip(QString("<b>%1 (%2)</b><br/> Stage current changes").arg(_SAVE->text()).arg(_SAVE->shortcut().toString()));

  _CANCEL->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_R));
  _CANCEL->setToolTip(QString("<b>%1 (%2)</b><br/> Cancel current changes").arg(_CANCEL->text()).arg(_CANCEL->shortcut().toString()));

  _SELECT_A_FOLDER_AND_LOAD_JSON->setShortcut(
      QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_O));
  _SELECT_A_FOLDER_AND_LOAD_JSON->setToolTip(QString("<b>%1 (%2)</b><br/> Select a folder and load json from it")
                                                 .arg(_SELECT_A_FOLDER_AND_LOAD_JSON->text())
                                                 .arg(_SELECT_A_FOLDER_AND_LOAD_JSON->shortcut().toString()));

  _RELOAD_JSON_FROM_FROM_DISK->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_F5));
  _RELOAD_JSON_FROM_FROM_DISK->setToolTip(QString("<b>%1 (%2)</b><br/> Reload json file from disk")
                                              .arg(_RELOAD_JSON_FROM_FROM_DISK->text())
                                              .arg(_RELOAD_JSON_FROM_FROM_DISK->shortcut().toString()));

  _AUTO_SKIP->setCheckable(true);
  _AUTO_SKIP->setChecked(true);
  _CONDITION_NOT->setCheckable(true);

  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_U));
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setToolTip(QString("<b>%1 (%2)</b><br/> Capitalize first letter of each word in a sentence.")
                                                        .arg(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->text())
                                                        .arg(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->shortcut().toString()));

  _LOWER_ALL_WORDS->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_L));
  _LOWER_ALL_WORDS->setToolTip(
      QString("<b>%1 (%2)</b><br/> Lowercase a sentense.").arg(_LOWER_ALL_WORDS->text()).arg(_LOWER_ALL_WORDS->shortcut().toString()));

  FILES_ACTIONS->addAction(_SELECT_A_FOLDER_AND_LOAD_JSON);
  FILES_ACTIONS->addAction(_EMPTY_JSONS_LISTWIDGET);
  FILES_ACTIONS->addAction(_LAST_JSON);
  FILES_ACTIONS->addAction(_NEXT_JSON);
  FILES_ACTIONS->addAction(_AUTO_SKIP);
  FILES_ACTIONS->addAction(_CONDITION_NOT);
  FILES_ACTIONS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  EDIT_ACTIONS->addAction(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD);
  EDIT_ACTIONS->addAction(_LOWER_ALL_WORDS);
  EDIT_ACTIONS->addAction(_SAVE);
  EDIT_ACTIONS->addAction(_CANCEL);
  EDIT_ACTIONS->addAction(_FORMATTER);
  EDIT_ACTIONS->addAction(_RELOAD_JSON_FROM_FROM_DISK);
  EDIT_ACTIONS->addAction(_ADD_SELECTED_PERFORMER);
  EDIT_ACTIONS->addAction(_SUBMIT);
  EDIT_ACTIONS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::AltModifier | Qt::Key_R));
  _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%1 (%2)</b><br/> Reveal the json in its parent folder.")
                                      .arg(_REVEAL_IN_EXPLORER->text())
                                      .arg(_REVEAL_IN_EXPLORER->shortcut().toString()));

  _OPEN_THIS_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _OPEN_THIS_FILE->setShortcutVisibleInContextMenu(true);
  _OPEN_THIS_FILE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Open this json file.").arg(_OPEN_THIS_FILE->text()).arg(_OPEN_THIS_FILE->shortcut().toString()));

  _HINT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_H));
  _HINT->setToolTip(QString("<b>%1 (%2)</b><br/> Give you performers list hint").arg(_HINT->text()).arg(_HINT->shortcut().toString()));

  _LEARN_PERFORMERS_FROM_JSON->setToolTip(
      QString("<b>%1 (%2)</b><br/> Learn performers from value of key \"Performers\" in json file. \n Improve its performers hint capability.")
          .arg(_LEARN_PERFORMERS_FROM_JSON->text())
          .arg(_LEARN_PERFORMERS_FROM_JSON->shortcut().toString()));

  LOAD_JSONS_FROM_CURRENT_FOLDER->setCheckable(false);
  LOAD_JSONS_FROM_CURRENT_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/>Load json files from current view root path.")
                                                 .arg(LOAD_JSONS_FROM_CURRENT_FOLDER->text(), LOAD_JSONS_FROM_CURRENT_FOLDER->shortcut().toString()));

  CONSTRUCT_JSONS_FOR_VIDS->setToolTip(QString("<b>%1 (%2)</b><br/>Construct json file for each vids under current path if json not exists.")
                                           .arg(CONSTRUCT_JSONS_FOR_VIDS->text(), CONSTRUCT_JSONS_FOR_VIDS->shortcut().toString()));

  JSON_ADD_PERFORMERS->setToolTip(QString("<b>%1 (%2)</b><br/>Input and Add performers to json file")
                                      .arg(JSON_ADD_PERFORMERS->text(), JSON_ADD_PERFORMERS->shortcut().toString()));

  JSON_SET_PRODUCTION_STUDIO->setToolTip(QString("<b>%1 (%2)</b><br/>Input and Set production name for json file")
                                             .arg(JSON_SET_PRODUCTION_STUDIO->text(), JSON_SET_PRODUCTION_STUDIO->shortcut().toString()));

  JSON_ADD_PERFORMER_KEY_VALUE_PAIR->setToolTip(
      QString("<b>%1 (%2)</b><br/>Add performers key-value pair for json file if key not exists. \n Otherwise init its values.")
          .arg(JSON_ADD_PERFORMER_KEY_VALUE_PAIR->text(), JSON_ADD_PERFORMER_KEY_VALUE_PAIR->shortcut().toString()));

  JSON_ADD_PRODUCTION_STUDIO_KEY_VALUE_PAIR->setToolTip(
      QString("<b>%1 (%2)</b><br/>Add production studio key-value pair for json file if key not exists. \n Otherwise init its values.")
          .arg(JSON_ADD_PRODUCTION_STUDIO_KEY_VALUE_PAIR->text(), JSON_ADD_PRODUCTION_STUDIO_KEY_VALUE_PAIR->shortcut().toString()));

  JSON_CLEAR_PERFORMERS_PROD_STUDIO_NAME->setToolTip(
      QString("<b>%1 (%2)</b><br/>Clear both performers and production studio values for json file.")
          .arg(JSON_CLEAR_PERFORMERS_PROD_STUDIO_NAME->text(), JSON_CLEAR_PERFORMERS_PROD_STUDIO_NAME->shortcut().toString()));

  BATCH_FILES_ACTIONS->addAction(LOAD_JSONS_FROM_CURRENT_FOLDER);
  BATCH_FILES_ACTIONS->addAction(CONSTRUCT_JSONS_FOR_VIDS);
  BATCH_FILES_ACTIONS->addAction(JSON_ADD_PERFORMERS);
  BATCH_FILES_ACTIONS->addAction(JSON_SET_PRODUCTION_STUDIO);
  BATCH_FILES_ACTIONS->addAction(JSON_ADD_PERFORMER_KEY_VALUE_PAIR);
  BATCH_FILES_ACTIONS->addAction(JSON_ADD_PRODUCTION_STUDIO_KEY_VALUE_PAIR);
  BATCH_FILES_ACTIONS->addAction(JSON_CLEAR_PERFORMERS_PROD_STUDIO_NAME);
}

JsonEditorActions& g_jsonEditorActions() {
  static JsonEditorActions ins;
  return ins;
}
