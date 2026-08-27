#ifndef MOVIEDBVIEW_H
#define MOVIEDBVIEW_H

#include "DatabaseSearchToolBar.h"
#include "FdBasedDb.h"
#include "FdBasedDbModel.h"
#include "CustomTableView.h"
#include "JsonModelField.h"
#include "MovieDBModelField.h"

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
  bool onScanFilesUnderPath(MovieDBModelField::ScanFilesTypeE filesType);
  bool onInitDataBase();
  bool onCreateATable();
  bool onDropATable();
  int onDeleteFromTable();

  bool onUnionTables();
  bool onAuditATable();
  bool onSetDurationByVideo();
  int onExportToJson();
  int onExportToEfuFile();
  int onUpdateByJson();

  int onCountRow();

  int onSetStudio();
  int onSetCastOrTags(const JsonModelField::FIELD_OP_TYPE type, const JsonModelField::FIELD_OP_MODE mode);

  QList<qint64> GetSelectionFileSizes() const;
  QList<int> GetSelectionDurations() const;


  // should not call ~destructure after getDb() and pass to QSqlTableModel
private:
  bool GetAPathFromUserSelect(const QString& usageMsg, QString& userSelected) const;
  bool IsHasSelection(const QString& msg = "") const;

  FdBasedDbModel* _dbModel{nullptr};
  MovieDBSearchToolBar* _movieDbSearchBar{nullptr};

  FdBasedDb& _fdBasedDb;

  bool m_isHeaderStateAlreadyInited{false};
  QStringList m_studioCandidates;
  QStringList m_candidatesLst[(int)JsonModelField::FIELD_OP_TYPE::BUTT];
};

#endif  // MOVIEDBVIEW_H
