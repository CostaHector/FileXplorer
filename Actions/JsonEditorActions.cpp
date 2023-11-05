#include "JsonEditorActions.h"

JsonEditorActions::JsonEditorActions(QObject* parent)
    : QObject{parent},
      _FORMATTER(new QAction(QIcon(":/themes/FORMAT"), "format", this)),
      _LAST(new QAction(QIcon(":/themes/LAST"), "last", this)),
      _NEXT(new QAction(QIcon(":/themes/NEXT"), "next", this)),
      _AUTO_SKIP(new QAction(QIcon(":/themes/AUTO_SKIP"), "auto skip", this)),
      _CONDITION_NOT(new QAction(QIcon(":/themes/CONDITION_NOT"), "!=", this)),
      _SUBMIT(new QAction(QIcon(":/themes/SUBMIT"), "submit", this)),
      _SAVE(new QAction(QIcon(":/themes/SAVED"), "save", this)),
      _CANCEL(new QAction(QIcon(":/themes/NOT_SAVED"), "cancel", this)),
      _LOAD(new QAction(QIcon(":/themes/LOAD_A_PATH"), "Load", this)),
      _EMPTY(new QAction(QIcon(":/themes/EMPTY_LISTWIDGET"), "Empty", this)),
      _RELOAD_FROM_JSON_FILE(new QAction(QIcon(":/themes/RELOAD_JOSN_FROM_FILE"), "Refresh", this)),
      JSON_EDITOR_ACTIONS(new QActionGroup(this)),
      _REVEAL_IN_EXPLORER(new QAction(QIcon(":/themes/REVEAL_IN_EXPLORER"), "reveal", this)),
      _OPEN_THIS_FILE(new QAction("open", this)),
      _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD(new QAction(QIcon(":/themes/CAPITALIZE_EACH_WORD"), "capitalize each word", this)),
      _HINT(new QAction(QIcon(":/themes/PERFORMERS_LIST_HINT"), "hint", this)),
      _LEARN_PERFORMERS_FROM_JSON(new QAction(QIcon(":/themes/LEARN_PERFORMERS_FROM_JSON"), "Learn performers", this)) {
  _FORMATTER->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_I));
  _FORMATTER->setToolTip(QString("<b>%1 (%2)</b><br/> Format current json(not stage). e.g., A,B -> A, B.")
                             .arg(_FORMATTER->text())
                             .arg(_FORMATTER->shortcut().toString()));

  _NEXT->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_Period));
  _NEXT->setToolTip(QString("<b>%1 (%2)</b><br/> Next one json(if exists)").arg(_NEXT->text()).arg(_NEXT->shortcut().toString()));

  _LAST->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_Comma));
  _LAST->setToolTip(QString("<b>%1 (%2)</b><br/> Last one json(if exists)").arg(_LAST->text()).arg(_LAST->shortcut().toString()));

  _SUBMIT->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_S));
  _SUBMIT->setToolTip(
      QString("<b>%1 (%2)</b><br/> (CANNOT RECOVER!) Submit all staged changes").arg(_SUBMIT->text()).arg(_SUBMIT->shortcut().toString()));

  _SAVE->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_S));
  _SAVE->setToolTip(QString("<b>%1 (%2)</b><br/> Stage current changes").arg(_SAVE->text()).arg(_SAVE->shortcut().toString()));

  _CANCEL->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_R));
  _CANCEL->setToolTip(QString("<b>%1 (%2)</b><br/> Cancel current changes").arg(_CANCEL->text()).arg(_CANCEL->shortcut().toString()));

  _LOAD->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_O));
  _LOAD->setToolTip(QString("<b>%1 (%2)</b><br/> Load json from a path.").arg(_LOAD->text()).arg(_LOAD->shortcut().toString()));

  _RELOAD_FROM_JSON_FILE->setShortcut(QKeySequence(Qt::KeyboardModifier::NoModifier | Qt::Key::Key_F5));
  _RELOAD_FROM_JSON_FILE->setToolTip(QString("<b>%1 (%2)</b><br/> Reload json file from local path.")
                                         .arg(_RELOAD_FROM_JSON_FILE->text())
                                         .arg(_RELOAD_FROM_JSON_FILE->shortcut().toString()));

  _AUTO_SKIP->setCheckable(true);
  _AUTO_SKIP->setChecked(true);
  _CONDITION_NOT->setCheckable(true);

  JSON_EDITOR_ACTIONS->addAction(_LOAD);
  JSON_EDITOR_ACTIONS->addAction(_EMPTY);

  JSON_EDITOR_ACTIONS->addAction(_LAST);
  JSON_EDITOR_ACTIONS->addAction(_NEXT);

  JSON_EDITOR_ACTIONS->addAction(_AUTO_SKIP);
  JSON_EDITOR_ACTIONS->addAction(_CONDITION_NOT);

  JSON_EDITOR_ACTIONS->addAction(_SAVE);
  JSON_EDITOR_ACTIONS->addAction(_CANCEL);
  JSON_EDITOR_ACTIONS->addAction(_FORMATTER);
  JSON_EDITOR_ACTIONS->addAction(_RELOAD_FROM_JSON_FILE);
  JSON_EDITOR_ACTIONS->addAction(_SUBMIT);

  JSON_EDITOR_ACTIONS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::AltModifier | Qt::Key_R));
  _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
  _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%1 (%2)</b><br/> Reveal the json in its parent folder.")
                                      .arg(_REVEAL_IN_EXPLORER->text())
                                      .arg(_REVEAL_IN_EXPLORER->shortcut().toString()));

  _OPEN_THIS_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  _OPEN_THIS_FILE->setShortcutVisibleInContextMenu(true);
  _OPEN_THIS_FILE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Open this json file.").arg(_OPEN_THIS_FILE->text()).arg(_OPEN_THIS_FILE->shortcut().toString()));


  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_U));
  _CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->setToolTip(QString("<b>%1 (%2)</b><br/> Capitalize first letter of each word").arg(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->text()).arg(_CAPITALIZE_FIRST_LETTER_OF_EACH_WORD->shortcut().toString()));

  _HINT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_H));
  _HINT->setToolTip(QString("<b>%1 (%2)</b><br/> Give you performers list hint").arg(_HINT->text()).arg(_HINT->shortcut().toString()));

  _LEARN_PERFORMERS_FROM_JSON->setToolTip(
      QString("<b>%1 (%2)</b><br/> Learn performers from value of key \"Performers\" in json file. \n Improve its performers hint capability.")
          .arg(_LEARN_PERFORMERS_FROM_JSON->text())
          .arg(_LEARN_PERFORMERS_FROM_JSON->shortcut().toString()));
}

JsonEditorActions& g_jsonEditorActions() {
  static JsonEditorActions ins;
  return ins;
}
