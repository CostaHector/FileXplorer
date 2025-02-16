#include "RenameActions.h"

RenameActions::RenameActions(QObject* parent) : QObject{parent} {
  _NUMERIZER = new QAction(QIcon(":img/NAME_STR_NUMERIZER_PATH"), "Rename (ith)");
  _SECTIONS_SWAPPER = new QAction(QIcon(":img/NAME_SECTIONS_SWAP"), "Sections Swap");
  _SWAP_2_NAMES = new QAction(QIcon(":img/RENAME_SWAP_2_NAMES"), "Swap 2 names");
  _CASE_NAME = new QAction(QIcon(":img/NAME_STR_CASE"), "Case Change");
  _STR_INSERTER = new QAction(QIcon(":img/NAME_STR_INSERTER_PATH"), "String Insert");
  _STR_DELETER = new QAction(QIcon(":img/NAME_STR_DELETER_PATH"), "String Delete");
  _STR_REPLACER = new QAction(QIcon(":img/NAME_STR_REPLACER_PATH"), "String Replace");
  _CONTINUOUS_NUMBERING = new QAction(QIcon(":img/_CONTINUOUS_NUMBERING"), "Name Continous");
  _CONVERT_UNICODE_TO_ASCII = new QAction(QIcon(":img/UNICODE_TO_ASCII_TEXT"), "Unicode Char to ASCII");
  RENAME_RIBBONS = Get_Rename_Actions();

  _UPPER_CASE = new QAction(QIcon(":img/RENAME_UPPER_CASE"), "Uppercase");
  _LOWER_CASE = new QAction(QIcon(":img/RENAME_LOWER_CASE"), "Lowercase");
  _SENTENSE_CASE = new QAction(QIcon(""), "Sentence Case");
  _SENTENSE_CASE_IGNORE = new QAction(QIcon(""), "Sentence Case(Ignore)");
  _SWAP_CASE = new QAction(QIcon(":img/RENAME_TOGGLE_CASE"), "Toggle Case");
  NAME_CASE = Get_CASE_Actions();
}

auto RenameActions::Get_CASE_Actions() -> QActionGroup* {
  _UPPER_CASE->setToolTip("All uppercase letters");
  _LOWER_CASE->setToolTip("All lowercase letters");
  _SWAP_CASE->setToolTip("Alternates between upper-and lower-case");
  _SENTENSE_CASE->setToolTip(
      "Capitalizes the first letter of each sentence while keeping all other letters in lower case, making it easy to read and understand.<br/>"
      "e.g.<br/>"
      "   i like NBA\tI Like Nba");
  _SENTENSE_CASE_IGNORE->setToolTip(
      "Capitalizes only the first letter of each sentence ignore others.<br/>"
      "   i like NBA\tI Like NBA");

  QActionGroup* caseAG = new QActionGroup(this);
  caseAG->addAction(_UPPER_CASE);
  caseAG->addAction(_LOWER_CASE);
  caseAG->addAction(_SENTENSE_CASE);
  caseAG->addAction(_SENTENSE_CASE_IGNORE);
  caseAG->addAction(_SWAP_CASE);
  caseAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
  for (QAction* act : caseAG->actions()) {
    act->setCheckable(true);
  }
  _SENTENSE_CASE_IGNORE->setChecked(true);
  return caseAG;
}

auto RenameActions::Get_Rename_Actions() -> QActionGroup* {
  _NUMERIZER->setShortcut(QKeySequence(Qt::Key_F2));
  _NUMERIZER->setToolTip(QString("<b>%1 (%2)</b><br/> Numerizer each file in a sequence.").arg(_NUMERIZER->text()).arg(_NUMERIZER->shortcut().toString()));

  _SECTIONS_SWAPPER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_F2));
  _SECTIONS_SWAPPER->setToolTip(QString("<b>%1 (%2)</b><br/> Swap sections in name. e.g., A-B-C -> A-C-B.").arg(_SECTIONS_SWAPPER->text()).arg(_SECTIONS_SWAPPER->shortcut().toString()));

  _SWAP_2_NAMES->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_R));
  _SWAP_2_NAMES->setToolTip(QString("<b>%1 (%2)</b><br/> Swap 2 filenames.<br/>"
                                    "e.g.,<br/>"
                                    "file1, file2 -> file2, file1")
                                .arg(_SWAP_2_NAMES->text())
                                .arg(_SWAP_2_NAMES->shortcut().toString()));

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

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(_NUMERIZER);
  actionGroup->addAction(_SECTIONS_SWAPPER);
  actionGroup->addAction(_SWAP_2_NAMES);
  actionGroup->addAction(_CASE_NAME);
  actionGroup->addAction(_STR_INSERTER);
  actionGroup->addAction(_STR_DELETER);
  actionGroup->addAction(_STR_REPLACER);
  actionGroup->addAction(_CONTINUOUS_NUMBERING);
  actionGroup->addAction(_CONVERT_UNICODE_TO_ASCII);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  for (QAction* act : actionGroup->actions()) {
    act->setCheckable(false);
    act->setShortcutVisibleInContextMenu(true);
  }
  return actionGroup;
}

RenameActions& g_renameAg() {
  static RenameActions renameActions;
  return renameActions;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QToolBar>
class RenameActionIllustration : public QToolBar {
 public:
  explicit RenameActionIllustration(const QString& title, QWidget* parent = nullptr) : QToolBar(title, parent) {
    addAction("start");
    addActions(g_renameAg().RENAME_RIBBONS->actions());
    addSeparator();
    addAction("seperator");
    addSeparator();
    addActions(g_renameAg().NAME_CASE->actions());
    addAction("end");
  }
};
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  RenameActionIllustration renameIllustration("Rename Items", nullptr);
  renameIllustration.show();
  return a.exec();
}
#endif
