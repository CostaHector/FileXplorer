#include "RenameActions.h"

auto RenameActions::Get_CASE_Actions() -> QActionGroup* {
  _LOOSE_CAPITALIZE->setToolTip(
      "Weak. Only capitalize first char of each word and skip others.<br/>e.g.<br/>"
      "   1. henry->Henry<br/>"
      "   2. HENRY->HENRY;USA->USA;");
  _STRICT_CAPITALIZE->setToolTip(
      "Strong. Capitalize first char of each word and lower others.<br/>e.g. <br/>"
      "   1. henry->Henry<br/>"
      "   2. HENRY->Henry; NBA->Nba");

  QActionGroup* caseAG = new QActionGroup(this);
  caseAG->addAction(_UPPER_CASE);
  caseAG->addAction(_LOWER_CASE);
  caseAG->addAction(_LOOSE_CAPITALIZE);
  caseAG->addAction(_STRICT_CAPITALIZE);
  caseAG->addAction(_SWAP_CASE);
  caseAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
  for (QAction* act : caseAG->actions()) {
    act->setCheckable(true);
  }
  _STRICT_CAPITALIZE->setChecked(true);
  return caseAG;
}

auto RenameActions::Get_Rename_Actions() -> QActionGroup* {
  _NUMERIZER->setShortcut(QKeySequence(Qt::Key_F2));
  _NUMERIZER->setToolTip(
      QString("<b>%1 (%2)</b><br/> Numerizer each file in a sequence.").arg(_NUMERIZER->text()).arg(_NUMERIZER->shortcut().toString()));

  _RENAME_SWAPPER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_F2));
  _RENAME_SWAPPER->setToolTip(QString("<b>%1 (%2)</b><br/> Swap sections in name. e.g., A-B-C -> A-C-B.")
                                  .arg(_RENAME_SWAPPER->text())
                                  .arg(_RENAME_SWAPPER->shortcut().toString()));

  _STR_INSERTER->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_I));
  _STR_INSERTER->setToolTip(
      QString("<b>%1 (%2)</b><br/> Insert a string into file name.").arg(_STR_INSERTER->text()).arg(_STR_INSERTER->shortcut().toString()));

  _STR_DELETER->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_D));
  _STR_DELETER->setToolTip(
      QString("<b>%1 (%2)</b><br/> Remove a substring from file name.").arg(_STR_DELETER->text(), _STR_DELETER->shortcut().toString()));

  _STR_REPLACER->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_R));
  _STR_REPLACER->setToolTip(QString("<b>%1 (%2)</b><br/> Replace a substring in file name with "
                                   "another string.")
                                .arg(_STR_REPLACER->text(), _STR_REPLACER->shortcut().toString()));

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(_NUMERIZER);
  actionGroup->addAction(_RENAME_SWAPPER);
  actionGroup->addAction(_CASE_NAME);
  actionGroup->addAction(_STR_INSERTER);
  actionGroup->addAction(_STR_DELETER);
  actionGroup->addAction(_STR_REPLACER);
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
