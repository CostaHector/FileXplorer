#ifndef CASTDBACTIONS_H
#define CASTDBACTIONS_H

#include <QAction>
#include <QMenuBar>
#include <QObject>
#include <QToolButton>
#include "PendingChangesAction.h"

class CastDBActions : public QObject {
  Q_OBJECT
 public:
  static CastDBActions& GetInst();

  PendingChangesAction* _MODEL_PENDING_CHANGES_ACTION{nullptr};

  QToolButton* GetAppendCastToolButton(QWidget* notNullParent);

  QAction* INSERT_A_ROW_BEFORE_CURRENT_INDEX{nullptr};
  QAction* APPEND_FROM_PSON_FILES{nullptr};
  QAction* APPEND_FROM_FILE_SYSTEM_STRUCTURE{nullptr};
  QAction* DELETE_RECORDS{nullptr};
  QAction* INIT_DATABASE{nullptr};
  QAction* INIT_TABLE{nullptr};
  QAction* DROP_TABLE{nullptr};
  QAction* DELETE_TABLE{nullptr};
  QActionGroup* BASIC_TABLE_OP{nullptr};

  QAction* SYNC_SELECTED_RECORDS_IMGS_FROM_DISK{nullptr};
  QAction* SYNC_ALL_RECORDS_IMGS_FROM_DISK{nullptr};

  QAction* SYNC_SELECTED_RECORDS_VIDS_FROM_DB{nullptr};
  QAction* SYNC_ALL_RECORDS_VIDS_FROM_DB{nullptr};

  QAction* DUMP_SELECTED_RECORDS_INTO_PSON_FILE{nullptr};
  QAction* DUMP_ALL_RECORDS_INTO_PSON_FILE{nullptr};

  QWidget* GetCastsDisplayMethodToolBar(QWidget* notNullParent);
  QAction* m_showCastsTableAct{nullptr};
  QAction* m_showCastsCardsListAct{nullptr};

  QAction* OPEN_DB_WITH_LOCAL_APP{nullptr};

private:
  explicit CastDBActions(QObject* parent = nullptr);
};

#endif  // CASTDBACTIONS_H
