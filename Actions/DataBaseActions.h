#ifndef DATABASEACTIONS_H
#define DATABASEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <QObject>

class DataBaseActions : public QObject {
  Q_OBJECT
 public:
  explicit DataBaseActions(QObject* parent = nullptr);

  QAction* SUBMIT{nullptr};
  QAction* INIT_A_DATABASE{nullptr};
  QAction* INIT_A_TABLE{nullptr};
  QAction* DROP_A_TABLE{nullptr};
  QAction* INSERT_A_PATH{nullptr};
  QAction* DELETE_FROM_TABLE{nullptr};
  QAction* UNION_TABLE{nullptr};
  QAction* AUDIT_A_TABLE{nullptr};
  QActionGroup* DB_CONTROL_ACTIONS{nullptr};

  QAction* SET_DURATION_BY_VIDEO{nullptr};
  QAction* EXPORT_DURATION_STUDIO_CAST_TAGS_TO_JSON{nullptr};
  QAction* UPDATE_STUDIO_CAST_TAGS_BY_JSON{nullptr};
  QActionGroup* EX_FUNCTION_ACTIONS{nullptr};

  QAction* QUICK_WHERE_CLAUSE{nullptr};
  QAction *_COUNT{nullptr}, *_SUM{nullptr};
};

DataBaseActions& g_dbAct();
#endif  // DATABASEACTIONS_H
