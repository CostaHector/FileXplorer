#ifndef CASTDBACTIONS_H
#define CASTDBACTIONS_H

#include <QAction>
#include <QMenuBar>
#include <QObject>

class CastDBActions : public QObject {
  Q_OBJECT
 public:
  explicit CastDBActions(QObject* parent = nullptr);

  QAction* REFRESH_ALL_RECORDS_VIDS;
  QAction* REFRESH_SELECTED_RECORDS_VIDS;
  QAction* OPEN_RECORD_IN_FILE_SYSTEM;

  QAction* LOAD_FROM_PJSON_PATH;
  QAction* LOAD_FROM_FILE_SYSTEM_STRUCTURE;
  QAction* LOAD_FROM_PERFORMERS_LIST;
  QAction* DUMP_ALL_RECORDS_INTO_PJSON_FILE;
  QAction* DUMP_SELECTED_RECORDS_INTO_PJSON_FILE;

  QAction* OPEN_DB_WITH_LOCAL_APP;
  QAction* LOCATE_IMAGEHOST;

  QAction* SUBMIT;
  QAction* INSERT_INTO_TABLE;
  QAction* INIT_DATABASE;
  QAction* INIT_TABLE;
  QAction* DROP_TABLE;
  QAction* DELETE_TABLE;

  QAction* CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT;

  QActionGroup* HORIZONTAL_HEADER_AGS;

  QAction* DELETE_RECORDS;
  QActionGroup* VERTICAL_HEADER_AGS;

  QAction* PERFORMERS_BOOK;

  QMenuBar* GetMenuBar() const;
  QMenu* GetRightClickMenu() const;

  QActionGroup* GetVerAGS() const;
  QActionGroup* GetHorAGS() const;
};

CastDBActions& g_performersManagerActions();

#endif  // CASTDBACTIONS_H
