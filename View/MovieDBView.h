#ifndef MOVIEDBVIEW_H
#define MOVIEDBVIEW_H

#include "DatabaseSearchToolBar.h"
#include "MovieDatabaseMenu.h"

#include "FdBasedDb.h"
#include "FdBasedDbModel.h"
#include "CustomTableView.h"
#include "JsonKey.h"

namespace MovieDBViewMock {

inline std::pair<bool, QString>& InputATableNameMock() {
  static std::pair<bool, QString> accept2TableNamePair;
  return accept2TableNamePair;
}
inline std::pair<bool, QString>& InputADeleteWhereClauseMock() {
  static std::pair<bool, QString> accept2deleteWhereClause;
  return accept2deleteWhereClause;
}
inline std::pair<bool, QString>& InputStudioNameMock() {
  static std::pair<bool, QString> accept2StudioName;
  return accept2StudioName;
}
inline bool& clearTagsOrCastsMock() {
  static bool bClearTagsOrCasts;
  return bClearTagsOrCasts;
}
inline std::pair<bool, QString>& InputTagsOrCastsMock() {
  static std::pair<bool, QString> accept2TagsOrCasts;
  return accept2TagsOrCasts;
}
inline QString& GetAPathFromFileDialogMock() {
  static QString pathFromFileDialog;
  return pathFromFileDialog;
}
inline bool& ConfirmInsertIntoMock() {
  static bool bConfirmInsertInto = false;
  return bConfirmInsertInto;
}
inline bool& ConfirmUnionIntoMock() {
  static bool bConfirmUnionInto = false;
  return bConfirmUnionInto;
}

inline void clear() {
  MovieDBViewMock::InputATableNameMock() = std::pair<bool, QString>(false, "");
  MovieDBViewMock::InputADeleteWhereClauseMock() = std::pair<bool, QString>(false, "");
  MovieDBViewMock::InputStudioNameMock() = std::pair<bool, QString>(false, "");
  MovieDBViewMock::clearTagsOrCastsMock() = false;
  MovieDBViewMock::InputTagsOrCastsMock() = std::pair<bool, QString>(false, "");
  MovieDBViewMock::GetAPathFromFileDialogMock() = "";
  MovieDBViewMock::ConfirmInsertIntoMock() = false;
  MovieDBViewMock::ConfirmUnionIntoMock() = false;
}

}

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
  bool GetAPathFromUserSelect(const QString& usageMsg, QString& userSelected);
  bool IsHasSelection(const QString& msg = "") const;

  FdBasedDbModel* _dbModel{nullptr};
  MovieDatabaseMenu* m_movieMenu{nullptr};
  MovieDBSearchToolBar* _movieDbSearchBar{nullptr};

  FdBasedDb& _fdBasedDb;

  QStringList m_studioCandidates;
  QStringList m_candidatesLst[(int)FIELD_OP_TYPE::BUTT];
};

#endif  // MOVIEDBVIEW_H
