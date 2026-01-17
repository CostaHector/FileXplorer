#ifndef PWDTABLEEDITACTIONS_H
#define PWDTABLEEDITACTIONS_H
#include <QAction>
#include <QActionGroup>
#include <QObject>

class PwdTableEditActions : public QObject {
  Q_OBJECT
public:
  explicit PwdTableEditActions(QObject *parent = nullptr);

  QAction *INSERT_A_ROW{nullptr}, *INSERT_ROWS{nullptr}, *APPEND_ROWS{nullptr};
  QAction *DELETE_ROWS{nullptr};
  QAction *LOAD_FROM_INPUT{nullptr};
  QActionGroup *ROW_EDIT_AG{nullptr};

  QAction *OPEN_DIRECTORY{nullptr};
  QAction *SHOW_PLAIN_CSV_CONTENT{nullptr};
  QAction *EXPORT_TO_PLAIN_CSV{nullptr};

  QAction *SEARCH_BY{nullptr};

  QAction *SAVE_CHANGES{nullptr};
};

PwdTableEditActions &GetTableEditActionsInst();

#endif // PWDTABLEEDITACTIONS_H
