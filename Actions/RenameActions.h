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
  QAction* _SECTIONS_ARRANGE {nullptr};
  QAction* _REVERSE_NAMES_LIST {nullptr};
  QAction* _CASE_NAME {nullptr};
  QAction* _STR_INSERTER {nullptr};
  QAction* _STR_DELETER {nullptr};
  QAction* _STR_REPLACER {nullptr};
  QAction* _CONTINUOUS_NUMBERING {nullptr};
  QAction* _CONVERT_UNICODE_TO_ASCII {nullptr};
  QAction* _PREPEND_PARENT_FOLDER_NAMES {nullptr};
  QActionGroup* RENAME_RIBBONS {nullptr};

  QAction* _UPPER_CASE {nullptr};
  QAction* _LOWER_CASE {nullptr};
  QAction* _CAPITALIZE_KEEP_OTHER {nullptr};
  QAction* _CAPITALIZE_LOWER_OTHER {nullptr};
  QAction* _TOGGLE_CASE {nullptr};
  QActionGroup* NAME_CASE {nullptr};
};

RenameActions& g_renameAg();
#endif  // RENAMEACTIONS_H
