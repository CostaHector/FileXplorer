#ifndef MOVIEDBVIEW_H
#define MOVIEDBVIEW_H

#include "DatabaseSearchToolBar.h"
#include "FdBasedDb.h"
#include "FdBasedDbModel.h"
#include "CustomTableView.h"
#include "JsonKey.h"

class MovieDBView : public CustomTableView {
public:
  MovieDBView(FdBasedDbModel* model_,              //
              MovieDBSearchToolBar* dbSearchBar,  //
              FdBasedDb& movieDb_,                 //
              QWidget* parent = nullptr);

  void subscribe();

  bool InitMoviesTables();
  bool setCurrentMovieTable(const QString& movieTableName);

  bool onSubmit();
  bool onRevert();
  bool onInsertIntoTable();
  bool onInitDataBase();
  bool onCreateATable();
  bool onDropATable();
  int onDeleteFromTable();

  bool onUnionTables();
  bool onAuditATable();
  bool onSetDurationByVideo();
  int onExportToJson();
  int onUpdateByJson();

  int onCountRow();

  int onSetStudio();
  int onSetCastOrTags(const FIELD_OP_TYPE type, const FIELD_OP_MODE mode);

  // should not call ~destructure after getDb() and pass to QSqlTableModel
private:
  bool GetAPathFromUserSelect(const QString& usageMsg, QString& userSelected) const;
  bool IsHasSelection(const QString& msg = "") const;

  FdBasedDbModel* _dbModel{nullptr};
  MovieDBSearchToolBar* _movieDbSearchBar{nullptr};

  FdBasedDb& _fdBasedDb;

  QStringList m_studioCandidates;
  QStringList m_candidatesLst[(int)FIELD_OP_TYPE::BUTT];
};

#endif  // MOVIEDBVIEW_H
