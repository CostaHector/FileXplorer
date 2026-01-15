#ifndef MOVIEDBACTIONS_H
#define MOVIEDBACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class MovieDBActions : public QObject {
  Q_OBJECT
 public:
  explicit MovieDBActions(QObject* parent = nullptr);

  QAction* SUBMIT{nullptr};
  QAction* REVERT{nullptr};
  QAction* INIT_A_DATABASE{nullptr};
  QAction* INIT_A_TABLE{nullptr};
  QAction* DROP_A_TABLE{nullptr};
  QAction* INSERT_A_PATH{nullptr};
  QAction* DELETE_FROM_TABLE{nullptr};
  QAction* UNION_TABLE{nullptr};
  QAction* AUDIT_A_TABLE{nullptr};
  QActionGroup* DB_CONTROL_ACTIONS{nullptr};

  QAction* READ_DURATION_BY_VIDEO{nullptr};
  QAction* EXPORT_DURATION_STUDIO_CAST_TAGS_TO_JSON{nullptr};
  QAction* UPDATE_STUDIO_CAST_TAGS_BY_JSON{nullptr};
  QActionGroup* EX_FUNCTION_ACTIONS{nullptr};

  QAction *_COUNT{nullptr}, *_SUM{nullptr}, *_OPEN_DB_WITH_LOCAL_APP{nullptr};

  QAction *SET_STUDIO{nullptr};
  QAction *SET_CAST{nullptr}, *APPEND_CAST{nullptr}, *REMOVE_CAST{nullptr};
  QAction *SET_TAGS{nullptr}, *APPEND_TAGS{nullptr}, *REMOVE_TAGS{nullptr};
};

MovieDBActions& g_dbAct();
#endif  // MOVIEDBACTIONS_H
