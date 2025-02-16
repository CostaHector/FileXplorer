#ifndef RENAMEACTIONS_H
#define RENAMEACTIONS_H

#include <QAction>
#include <QActionGroup>

class RenameActions : public QObject {
  Q_OBJECT
 public:
  explicit RenameActions(QObject* parent = nullptr);

  auto Get_CASE_Actions() -> QActionGroup*;
  auto Get_Rename_Actions() -> QActionGroup*;

  QAction* _NUMERIZER {nullptr};
  QAction* _SECTIONS_SWAPPER {nullptr};
  QAction* _SWAP_2_NAMES {nullptr};
  QAction* _CASE_NAME {nullptr};
  QAction* _STR_INSERTER {nullptr};
  QAction* _STR_DELETER {nullptr};
  QAction* _STR_REPLACER {nullptr};
  QAction* _CONTINUOUS_NUMBERING {nullptr};
  QAction* _CONVERT_UNICODE_TO_ASCII {nullptr};
  QActionGroup* RENAME_RIBBONS {nullptr};

  QAction* _UPPER_CASE {nullptr};
  QAction* _LOWER_CASE {nullptr};
  QAction* _SENTENSE_CASE {nullptr};
  QAction* _SENTENSE_CASE_IGNORE {nullptr};
  QAction* _SWAP_CASE {nullptr};
  QActionGroup* NAME_CASE {nullptr};
};

RenameActions& g_renameAg();
#endif  // RENAMEACTIONS_H
