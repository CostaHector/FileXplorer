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
  QAction* _NUMERIZER = new QAction(QIcon(":/themes/NAME_STR_NUMERIZER_PATH"), tr("Rename (ith)"));
  _NUMERIZER->setShortcut(QKeySequence(Qt::Key_F2));
  _NUMERIZER->setToolTip(
      QString("<b>%1 (%2)</b><br/> Numerizer each file in a sequence.").arg(_NUMERIZER->text()).arg(_NUMERIZER->shortcut().toString()));

  QAction* _RENAME_SWAPPER = new QAction(QIcon(":/themes/NAME_STR_SWAPPER_PATH"), tr("swap 1-2-3 to 1-3-2"));
  _RENAME_SWAPPER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_F2));
  _RENAME_SWAPPER->setToolTip(QString("<b>%1 (%2)</b><br/> Swap sections in name. e.g., A-B-C -> A-C-B.")
                                  .arg(_RENAME_SWAPPER->text())
                                  .arg(_RENAME_SWAPPER->shortcut().toString()));

  QAction* _caseName = new QAction(QIcon(":/themes/NAME_STR_CASE"), tr("Case"));

  QAction* _strInserter = new QAction(QIcon(":/themes/NAME_STR_INSERTER_PATH"), tr("Str Inserter"));
  _strInserter->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_I));
  _strInserter->setToolTip(
      QString("<b>%1 (%2)</b><br/> Insert a string into file name.").arg(_strInserter->text()).arg(_strInserter->shortcut().toString()));

  QAction* _strDeleter = new QAction(QIcon(":/themes/NAME_STR_DELETER_PATH"), tr("Str Deleter"));
  _strDeleter->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_D));
  _strDeleter->setToolTip(
      QString("<b>%1 (%2)</b><br/> Remove a substring from file name.").arg(_strDeleter->text(), _strDeleter->shortcut().toString()));

  QAction* _strReplacer = new QAction(QIcon(":/themes/NAME_STR_REPLACER_PATH"), tr("Str Replacer"));
  _strReplacer->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_R));
  _strReplacer->setToolTip(QString("<b>%1 (%2)</b><br/> Replace a substring in file name with "
                                   "another string.")
                               .arg(_strReplacer->text(), _strReplacer->shortcut().toString()));

  QActionGroup* actionGroup = new QActionGroup(this);
  actionGroup->addAction(_NUMERIZER);
  actionGroup->addAction(_RENAME_SWAPPER);
  actionGroup->addAction(_caseName);
  actionGroup->addAction(_strInserter);
  actionGroup->addAction(_strDeleter);
  actionGroup->addAction(_strReplacer);
  actionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  for (QAction* act : actionGroup->actions()) {
    act->setCheckable(false);
    act->setShortcutVisibleInContextMenu(true);
  };
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
