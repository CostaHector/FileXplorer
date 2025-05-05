#ifndef CASTDBACTIONS_H
#define CASTDBACTIONS_H

#include <QAction>
#include <QMenuBar>
#include <QObject>

class CastDBActions : public QObject {
  Q_OBJECT
 public:
  explicit CastDBActions(QObject* parent = nullptr);

  QAction* SUBMIT{nullptr};
  QAction* INSERT_INTO_TABLE{nullptr};
  QAction* DELETE_RECORDS{nullptr};
  QAction* INIT_DATABASE{nullptr};
  QAction* INIT_TABLE{nullptr};
  QAction* DROP_TABLE{nullptr};
  QAction* DELETE_TABLE{nullptr};
  QActionGroup* BASIC_TABLE_OP{nullptr};

  QAction* REFRESH_ALL_RECORDS_VIDS{nullptr};
  QAction* REFRESH_SELECTED_RECORDS_VIDS{nullptr};
  QActionGroup* REFRESH_OP{nullptr};

  QAction* OPEN_DB_WITH_LOCAL_APP{nullptr};
  QAction* OPEN_RECORD_IN_FILE_SYSTEM{nullptr};
  QAction* LOCATE_IMAGEHOST{nullptr};
  QActionGroup* FILE_SYSTEM_OP{nullptr};

  QAction* LOAD_FROM_PJSON_PATH{nullptr};
  QAction* LOAD_FROM_FILE_SYSTEM_STRUCTURE{nullptr};
  QAction* LOAD_FROM_PERFORMERS_LIST{nullptr};
  QActionGroup* LOAD_EXTEND_OP{nullptr};

  QAction* DUMP_ALL_RECORDS_INTO_PJSON_FILE{nullptr};
  QAction* DUMP_SELECTED_RECORDS_INTO_PJSON_FILE{nullptr};
  QActionGroup* EXPORT_OP{nullptr};

  QAction* CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT{nullptr};

  QActionGroup* HORIZONTAL_HEADER_AGS{nullptr};
  QActionGroup* VERTICAL_HEADER_AGS{nullptr};

  QMenuBar* GetMenuBar() const;
  QMenu* GetRightClickMenu() const;

  QActionGroup* GetVerAGS() const;
  QActionGroup* GetHorAGS() const;
};

CastDBActions& g_castAct();

#endif  // CASTDBACTIONS_H
