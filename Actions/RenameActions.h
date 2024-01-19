#ifndef RENAMEACTIONS_H
#define RENAMEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QHash>

class RenameActions : public QObject {
 public:
  explicit RenameActions(QObject* parent = nullptr);

  auto Get_CASE_Actions() -> QActionGroup* {
    QAction* UPPER_CASE = new QAction(QIcon(":/themes/RENAME_UPPER_CASE"), "Upper Case", this);
    QAction* LOWER_CASE = new QAction(QIcon(":/themes/RENAME_LOWER_CASE"), "Lower Case", this);
    QAction* LOOSE_CAPITALIZE = new QAction(QIcon(":/themes/RENAME_LOOSE_CAPITALIZE_CASE"), "Loose Capitalize", this);
    LOOSE_CAPITALIZE->setToolTip(
        "Weak. Only capitalize first char of each word and skip others.<br/>e.g.<br/>"
        "   1. henry->Henry<br/>"
        "   2. HENRY->HENRY;USA->USA;");
    QAction* STRICT_CAPITALIZE = new QAction(QIcon(":/themes/RENAME_STRICT_CAPITALIZE_CASE"), "Strict Capitalize", this);
    STRICT_CAPITALIZE->setToolTip(
        "Strong. Capitalize first char of each word and lower others.<br/>e.g. <br/>"
        "   1. henry->Henry<br/>"
        "   2. HENRY->Henry; NBA->Nba");
    QAction* SWAP_CASE = new QAction(QIcon(":/themes/RENAME_TOGGLE_CASE"), "Swap Case", this);

    QActionGroup* caseAG = new QActionGroup(this);
    caseAG->addAction(UPPER_CASE);
    caseAG->addAction(LOWER_CASE);
    caseAG->addAction(LOOSE_CAPITALIZE);
    caseAG->addAction(STRICT_CAPITALIZE);
    caseAG->addAction(SWAP_CASE);
    caseAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
    for (QAction* act : caseAG->actions()) {
      act->setCheckable(true);
    }
    STRICT_CAPITALIZE->setChecked(true);
    return caseAG;
  }
  auto Get_Rename_Actions() -> QActionGroup* {
    QAction* _NUMERIZER = new QAction(QIcon(":/themes/NAME_STR_NUMERIZER_PATH"), "Rename (ith)", this);
    _NUMERIZER->setShortcut(QKeySequence(Qt::Key_F2));
    _NUMERIZER->setToolTip(
        QString("<b>%1 (%2)</b><br/> Numerizer each file in a sequence.").arg(_NUMERIZER->text()).arg(_NUMERIZER->shortcut().toString()));

    QAction* _RENAME_SWAPPER = new QAction(QIcon(":/themes/NAME_STR_SWAPPER_PATH"), "swap 1-2-3 to 1-3-2", this);
    _RENAME_SWAPPER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_F2));
    _RENAME_SWAPPER->setToolTip(QString("<b>%1 (%2)</b><br/> Swap sections in name. e.g., A-B-C -> A-C-B.")
                                    .arg(_RENAME_SWAPPER->text())
                                    .arg(_RENAME_SWAPPER->shortcut().toString()));

    QAction* _caseName = new QAction(QIcon(":/themes/NAME_STR_CASE"), "Case", this);

    QAction* _strInserter = new QAction(QIcon(":/themes/NAME_STR_INSERTER_PATH"), "Str Inserter", this);
    _strInserter->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_I));
    _strInserter->setToolTip(
        QString("<b>%1 (%2)</b><br/> Insert a string into file name.").arg(_strInserter->text()).arg(_strInserter->shortcut().toString()));

    QAction* _strDeleter = new QAction(QIcon(":/themes/NAME_STR_DELETER_PATH"), "Str Deleter", this);
    _strDeleter->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::KeyboardModifier::ShiftModifier | Qt::Key::Key_D));
    _strDeleter->setToolTip(
        QString("<b>%1 (%2)</b><br/> Remove a substring from file name.").arg(_strDeleter->text(), _strDeleter->shortcut().toString()));

    QAction* _strReplacer = new QAction(QIcon(":/themes/NAME_STR_REPLACER_PATH"), "Str Replacer", this);
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
  QActionGroup* RENAME_RIBBONS;
  QActionGroup* NAME_CASE;
};

RenameActions& g_renameAg();
#endif  // RENAMEACTIONS_H
