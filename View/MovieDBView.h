#ifndef MOVIEDBVIEW_H
#define MOVIEDBVIEW_H

#include "DatabaseSearchToolBar.h"
#include "FdBasedDb.h"
#include "FdBasedDbModel.h"
#include "CustomTableView.h"
#include "JsonModelField.h"
#include "MovieDBModelField.h"

class MovieDBView : public CustomTableView {
  Q_OBJECT
public:
  MovieDBView(FdBasedDb& movieDb_,
              FdBasedDbModel* model_,              //
              MovieDBSearchToolBar* dbSearchBar_,  //
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

  bool onReconstructTotalMovieTable();
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

signals:
  void currentRecordChanged(const QSqlRecord& newRecord);

  // should not call ~destructure after getDb() and pass to QSqlTableModel
private:
  void initExclusivePreferenceSetting() override;
  void EmitCurrentCastRecordChanged(const QModelIndex& current, const QModelIndex& /*previous*/);

  static QString GetAPathFromUserSelect(const QString& curTblName, const QString& usageMsg);
  bool IsHasSelection(const QString& msg = "") const;

  FdBasedDb& _fdBasedDb;
  FdBasedDbModel* _dbModel{nullptr};
  MovieDBSearchToolBar* _movieDbSearchBar{nullptr};

  bool m_isHeaderStateAlreadyInited{false};
  QStringList m_studioCandidates;
  QStringList m_candidatesLst[(int)JsonModelField::FIELD_OP_TYPE::BUTT];
};

#endif  // MOVIEDBVIEW_H
