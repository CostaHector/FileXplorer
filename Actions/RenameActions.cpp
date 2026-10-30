#include "RenameActions.h"

RenameActions::RenameActions(QObject* parent) : QObject{parent} {
  _NUMERIZER = new (std::nothrow) QAction(QIcon(":img/NAME_STR_NUMERIZER_PATH"), tr("Rename (ith)"), this);
  _SECTIONS_ARRANGE = new (std::nothrow) QAction(QIcon(":img/NAME_SECTIONS_ARRANGE"), tr("Sections Arrange"), this);
  _REVERSE_NAMES_LIST = new (std::nothrow) QAction(QIcon(":img/RENAME_REVERSE_NAMES_LIST"), tr("Reverse names list"), this);
  _CASE_NAME = new (std::nothrow) QAction(QIcon(":img/NAME_STR_CASE"), tr("Case Change"), this);
  _STR_INSERTER = new (std::nothrow) QAction(QIcon(":img/NAME_STR_INSERTER_PATH"), tr("String Insert"), this);
  _STR_DELETER = new (std::nothrow) QAction(QIcon(":img/NAME_STR_DELETER_PATH"), tr("String Delete"), this);
  _STR_REPLACER = new (std::nothrow) QAction(QIcon(":img/NAME_STR_REPLACER_PATH"), tr("String Replace"), this);
  _CONTINUOUS_NUMBERING = new (std::nothrow) QAction(QIcon(":img/_CONTINUOUS_NUMBERING"), tr("Name Continous"), this);
  _CONVERT_UNICODE_TO_ASCII = new (std::nothrow) QAction(QIcon(":img/UNICODE_TO_ASCII_TEXT"), tr("Unicode Char to ASCII"), this);
  _PREPEND_PARENT_FOLDER_NAMES = new (std::nothrow) QAction{QIcon{":img/PREPEND_PARENT_FOLDER_NAMES"}, tr("Prepend parent folder names"), this};
  _DIGIT_CHAR_REPLACE_TO_ALPHA = new (std::nothrow) QAction{QIcon{":img/DIGIT_2_ALPHA"}, tr("Digit 2 Alpha"), this};
  RENAME_RIBBONS = Get_Rename_Actions();
}

QActionGroup* RenameActions::Get_Rename_Actions() {
  _NUMERIZER->setShortcut(QKeySequence(Qt::Key_F2));
  _NUMERIZER->setToolTip(QString("<b>%1 (%2)</b><br/> Numerizer each file in a sequence.").arg(_NUMERIZER->text()).arg(_NUMERIZER->shortcut().toString()));

  _SECTIONS_ARRANGE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_F2));
  _SECTIONS_ARRANGE->setToolTip(QString("<b>%1 (%2)</b><br/> Swap sections in name. e.g., A-B-C -> A-C-B.").arg(_SECTIONS_ARRANGE->text()).arg(_SECTIONS_ARRANGE->shortcut().toString()));

  _REVERSE_NAMES_LIST->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_R));
  _REVERSE_NAMES_LIST->setToolTip(QString("<b>%1 (%2)</b><br/> Swap 2 filenames.<br/>"
                                          "e.g.,<br/>"
                                          "file1, file2 -> file2, file1")
                                  .arg(_REVERSE_NAMES_LIST->text())
                                  .arg(_REVERSE_NAMES_LIST->shortcut().toString()));

  _STR_INSERTER->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_I));
  _STR_INSERTER->setToolTip(QString("<b>%1 (%2)</b><br/> Insert a string into file name.").arg(_STR_INSERTER->text()).arg(_STR_INSERTER->shortcut().toString()));

  _STR_DELETER->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_D));
  _STR_DELETER->setToolTip(QString("<b>%1 (%2)</b><br/> Remove a substring from file name.").arg(_STR_DELETER->text(), _STR_DELETER->shortcut().toString()));

  _STR_REPLACER->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_R));
  _STR_REPLACER->setToolTip(QString("<b>%1 (%2)</b><br/> Replace a substring in file name with "
                                    "another string.")
                            .arg(_STR_REPLACER->text(), _STR_REPLACER->shortcut().toString()));

  _CONTINUOUS_NUMBERING->setToolTip(QString("<b>%1 (%2)</b><br/> Make file number Continuous.").arg(_CONTINUOUS_NUMBERING->text(), _CONTINUOUS_NUMBERING->shortcut().toString()));

  _CONVERT_UNICODE_TO_ASCII->setToolTip(
        QString("<b>%1 (%2)</b><br/> Convert unicode charset in name to ascii").arg(_CONVERT_UNICODE_TO_ASCII->text(), _CONVERT_UNICODE_TO_ASCII->shortcut().toString()));

  _PREPEND_PARENT_FOLDER_NAMES->setToolTip(
        QString("<b>%1 (%2)</b><br/> Prepend parent folder name to files under user selected folders").arg(_PREPEND_PARENT_FOLDER_NAMES->text(), _PREPEND_PARENT_FOLDER_NAMES->shortcut().toString()));

  _DIGIT_CHAR_REPLACE_TO_ALPHA->setToolTip(
      QString("<b>%1 (%2)</b><br/> Replace 4:a, 0:o, and, 1:i").arg(_DIGIT_CHAR_REPLACE_TO_ALPHA->text(), _DIGIT_CHAR_REPLACE_TO_ALPHA->shortcut().toString()));

  QActionGroup* actionGroup = new (std::nothrow) QActionGroup(this);
  actionGroup->addAction(_NUMERIZER);
  actionGroup->addAction(_SECTIONS_ARRANGE);
  actionGroup->addAction(_REVERSE_NAMES_LIST);
  actionGroup->addAction(_CASE_NAME);
  actionGroup->addAction(_STR_INSERTER);
  actionGroup->addAction(_STR_DELETER);
  actionGroup->addAction(_STR_REPLACER);
  actionGroup->addAction(_CONTINUOUS_NUMBERING);
  actionGroup->addAction(_CONVERT_UNICODE_TO_ASCII);
  actionGroup->addAction(_PREPEND_PARENT_FOLDER_NAMES);
  actionGroup->addAction(_DIGIT_CHAR_REPLACE_TO_ALPHA);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  foreach(QAction* act, actionGroup->actions()) {
    act->setCheckable(false);
    act->setShortcutVisibleInContextMenu(true);
  }
  return actionGroup;
}

RenameActions& g_renameAg() {
  static RenameActions renameActions;
  return renameActions;
}
