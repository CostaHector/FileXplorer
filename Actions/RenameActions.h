#ifndef RENAMEACTIONS_H
#define RENAMEACTIONS_H

#include <QAction>
#include <QActionGroup>

class RenameActions : public QObject {
  Q_OBJECT
 public:
  explicit RenameActions(QObject* parent = nullptr);

  QActionGroup* Get_Rename_Actions();

  QList<QAction*> GetGeneralRenameActions() {
    return {_STR_INSERTER, _STR_DELETER, _STR_REPLACER};
  }
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
  QAction* _DIGIT_CHAR_REPLACE_TO_ALPHA{nullptr};
  QActionGroup* RENAME_RIBBONS {nullptr};
};

RenameActions& g_renameAg();
#endif  // RENAMEACTIONS_H
