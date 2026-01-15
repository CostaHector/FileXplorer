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
  RENAME_RIBBONS = Get_Rename_Actions();

  _UPPER_CASE = new (std::nothrow) QAction(QIcon(":img/RENAME_UPPER_CASE"), tr("Uppercase"), this);
  _LOWER_CASE = new (std::nothrow) QAction(QIcon(":img/RENAME_LOWER_CASE"), tr("Lowercase"), this);
  _CAPITALIZE_KEEP_OTHER = new (std::nothrow) QAction(QIcon(":img/CAPITALIZE_KEEP_OTHER"), tr("Capitalize first and keep other"), this);
  _CAPITALIZE_LOWER_OTHER = new (std::nothrow) QAction(QIcon(":img/CAPITALIZE_LOWER_OTHER"), tr("Capitalize first and lower other"), this);
  _TOGGLE_CASE = new (std::nothrow) QAction(QIcon(":img/RENAME_TOGGLE_CASE"), tr("Toggle Case"), this);
  NAME_CASE = Get_CASE_Actions();
}

auto RenameActions::Get_CASE_Actions() -> QActionGroup* {
  _UPPER_CASE->setToolTip("All uppercase letters");
  _LOWER_CASE->setToolTip("All lowercase letters");
  _TOGGLE_CASE->setToolTip("Alternates between upper-and lower-case");
  _CAPITALIZE_KEEP_OTHER->setToolTip(
        "Capitalizes first letter of each sentence and ignore other letters.<br/>"
        "Making it easy to read and understand.<br/>"
        "e.g.<br/>"
        "i like NBA\tI Like NBA");
  _CAPITALIZE_LOWER_OTHER->setToolTip(
        "Capitalizes first letter of each sentence and lower others.<br/>"
        "e.g.<br/>"
        "i like NBA\tI Like Nba");

  QActionGroup* caseAG = new (std::nothrow) QActionGroup(this);
  caseAG->addAction(_UPPER_CASE);
  caseAG->addAction(_LOWER_CASE);
  caseAG->addAction(_CAPITALIZE_KEEP_OTHER);
  caseAG->addAction(_CAPITALIZE_LOWER_OTHER);
  caseAG->addAction(_TOGGLE_CASE);
  caseAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
  foreach(QAction* act, caseAG->actions()) {
    act->setCheckable(true);
  }
  _CAPITALIZE_LOWER_OTHER->setChecked(true);
  return caseAG;
}

auto RenameActions::Get_Rename_Actions() -> QActionGroup* {
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
