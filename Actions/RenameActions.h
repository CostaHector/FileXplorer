#ifndef RENAMEACTIONS_H
#define RENAMEACTIONS_H

#include <QAction>
#include <QActionGroup>

class RenameActions : public QObject {
  Q_OBJECT
 public:
  explicit RenameActions(QObject* parent = nullptr)
      : QObject{parent},
        _NUMERIZER{new QAction(QIcon(":/themes/NAME_STR_NUMERIZER_PATH"), tr("Rename (ith)"))},
        _RENAME_SWAPPER{new QAction(QIcon(":/themes/NAME_STR_SWAPPER_PATH"), tr("swap 1-2-3 to 1-3-2"))},
        _CASE_NAME{new QAction(QIcon(":/themes/NAME_STR_CASE"), tr("Case"))},
        _STR_INSERTER{new QAction(QIcon(":/themes/NAME_STR_DELETER_PATH"), tr("Str Inserter"))},
        _STR_DELETER{new QAction(QIcon(":/themes/NAME_STR_DELETER_PATH"), tr("Str Deleter"))},
        _STR_REPLACER{new QAction(QIcon(":/themes/NAME_STR_REPLACER_PATH"), tr("Str Replacer"))},
        _NAME_NO_CONSECUTIVE{new QAction(QIcon(":/themes/_NAME_NO_CONSECUTIVE"), tr("Name No Consecutive"))},
        _CONVERT_UNICODE_TO_ASCII{new QAction(QIcon(":/themes/UNICODE_TO_ASCII_TEXT"), tr("Unicode to ASCII"))},
        RENAME_RIBBONS(Get_Rename_Actions()),
        _UPPER_CASE{new QAction(QIcon(":/themes/RENAME_UPPER_CASE"), tr("Upper Case"))},
        _LOWER_CASE{new QAction(QIcon(":/themes/RENAME_LOWER_CASE"), tr("Lower Case"))},
        _LOOSE_CAPITALIZE{new QAction(QIcon(":/themes/RENAME_LOOSE_CAPITALIZE_CASE"), tr("Loose Capitalize"))},
        _STRICT_CAPITALIZE{new QAction(QIcon(":/themes/RENAME_STRICT_CAPITALIZE_CASE"), tr("Strict Capitalize"))},
        _SWAP_CASE{new QAction(QIcon(":/themes/RENAME_TOGGLE_CASE"), tr("Swap Case"))},
        NAME_CASE(Get_CASE_Actions()) {}

  auto Get_CASE_Actions() -> QActionGroup*;
  auto Get_Rename_Actions() -> QActionGroup*;

  QAction* _NUMERIZER;
  QAction* _RENAME_SWAPPER;
  QAction* _CASE_NAME;
  QAction* _STR_INSERTER;
  QAction* _STR_DELETER;
  QAction* _STR_REPLACER;
  QAction* _NAME_NO_CONSECUTIVE;
  QAction* _CONVERT_UNICODE_TO_ASCII;
  QActionGroup* RENAME_RIBBONS;

  QAction* _UPPER_CASE;
  QAction* _LOWER_CASE;
  QAction* _LOOSE_CAPITALIZE;
  QAction* _STRICT_CAPITALIZE;
  QAction* _SWAP_CASE;
  QActionGroup* NAME_CASE;
};

RenameActions& g_renameAg();
#endif  // RENAMEACTIONS_H
