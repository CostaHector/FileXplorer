#ifndef RENAMEACTIONS_H
#define RENAMEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QHash>

class RenameActions : public QObject {
  Q_OBJECT
 public:
  explicit RenameActions(QObject* parent = nullptr)
      : QObject{parent},
        RENAME_RIBBONS(Get_Rename_Actions()),
        _UPPER_CASE{new QAction(QIcon(":/themes/RENAME_UPPER_CASE"), tr("Upper Case"))},
        _LOWER_CASE{new QAction(QIcon(":/themes/RENAME_LOWER_CASE"), tr("Lower Case"))},
        _LOOSE_CAPITALIZE{new QAction(QIcon(":/themes/RENAME_LOOSE_CAPITALIZE_CASE"), tr("Loose Capitalize"))},
        _STRICT_CAPITALIZE{new QAction(QIcon(":/themes/RENAME_STRICT_CAPITALIZE_CASE"), tr("Strict Capitalize"))},
        _SWAP_CASE{new QAction(QIcon(":/themes/RENAME_TOGGLE_CASE"), tr("Swap Case"))},
        NAME_CASE(Get_CASE_Actions()) {}

  auto Get_CASE_Actions() -> QActionGroup*;
  auto Get_Rename_Actions() -> QActionGroup*;

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
