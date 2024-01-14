#ifndef PERFORMERSMANAGERACTIONS_H
#define PERFORMERSMANAGERACTIONS_H

#include <QAction>
#include <QMenuBar>
#include <QObject>

class PerformersManagerActions : public QObject {
  Q_OBJECT
 public:
  explicit PerformersManagerActions(QObject* parent = nullptr);

  QAction* REFRESH_ALL_RECORDS_VIDS;
  QAction* REFRESH_SELECTED_RECORDS_VIDS;
  QAction* OPEN_RECORD_IN_FILE_SYSTEM;

  QAction* LOAD_FROM_PJSON_PATH;
  QAction* LOAD_FROM_FILE_SYSTEM_STRUCTURE;
  QAction* LOAD_FROM_PERFORMERS_LIST;
  QAction* DUMP_ALL_RECORDS_INTO_PJSON_FILE;
  QAction* DUMP_SELECTED_RECORDS_INTO_PJSON_FILE;
  QAction* OPEN_WITH_LOCAL_APP;
  QAction* LOCATE_IMAGEHOST;

  QAction* INIT_DATABASE;
  QAction* INIT_TABLE;
  QAction* INSERT_INTO_TABLE;

  QAction* DELETE_TABLE;
  QAction* DROP_TABLE;
  QAction* SUBMIT;

  QAction* COLUMNS_VISIBILITY;
  QAction* CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT;

  QAction* HIDE_THIS_COLUMN;
  QAction* SHOW_ALL_COLUMNS;
  QAction* STRETCH_DETAIL_SECTION;
  QActionGroup* HORIZONTAL_HEADER_AGS;

  QAction* DELETE_RECORDS;
  QAction* RESIZE_ROWS_TO_CONTENT;
  QAction* RESIZE_ROWS_DEFAULT_SECTION_SIZE;
  QActionGroup* VERTICAL_HEADER_AGS;

  QAction* SHOW_PERFORMER_MANAGER;

  QMenuBar* m_menuBar;

 private:
  QMenuBar* GetMenuBar() const;

  void InitActionsTooltips();
};

PerformersManagerActions& g_performersManagerActions();

#endif  // PERFORMERSMANAGERACTIONS_H
