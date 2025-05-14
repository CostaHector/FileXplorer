#ifndef MOVIEDBVIEW_H
#define MOVIEDBVIEW_H

#include "Component/DatabaseSearchToolBar.h"
#include "Component/MovieDatabaseMenu.h"
#include "Component/QuickWhereClause.h"

#include "Tools/FileDescriptor/FdBasedDb.h"
#include "Model/FdBasedDbModel.h"
#include "View/CustomTableView.h"
#include "Tools/Json/JsonKey.h"

class MovieDBView : public CustomTableView {
 public:
  MovieDBView(FdBasedDbModel* model_,              //
              DatabaseSearchToolBar* dbSearchBar,  //
              FdBasedDb& movieDb_,                 //
              QWidget* parent = nullptr);

  void subscribe();

  bool onSearchDataBase();

  bool InitMoviesTables();
  bool setCurrentMovieTable(const QString& movieTableName);

  bool onSubmit();
  bool onRevert();
  bool onInsertIntoTable();
  bool onInitDataBase();
  void onCreateATable();
  bool onDropATable();
  bool onDeleteFromTable();

  bool onUnionTables();
  bool onAuditATable();
  bool onSetDurationByVideo();
  bool onExportToJson();
  bool onUpdateByJson();

  bool onQuickWhereClause();
  int onCountRow();

  QString GetMovieTableName() const {  // Table Name
    if (_tablesDropDownList == nullptr) {
      qWarning("_tablesDropDownList is nullptr");
      return "";
    }
    return _tablesDropDownList->CurrentTableName();
  }
  QString GetMovieTableRootPath() const {  // Peer Path
    if (_tablesDropDownList == nullptr) {
      qWarning("_tablesDropDownList is nullptr");
      return "";
    }
    return _tablesDropDownList->CurrentRootPath();
  }

  int onSetStudio();
  int onSetCastOrTags(const FIELD_OP_TYPE type, const FIELD_OP_MODE mode);
  // should not call ~destructure after getDb() and pass to QSqlTableModel
 private:
  bool GetAPathFromUserSelect(const QString& usageMsg, QString& userSelected);
  bool IsHasSelection(const QString& msg = "") const;

  FdBasedDbModel* _dbModel{nullptr};
  MovieDatabaseMenu* m_movieMenu{nullptr};
  Guid2RootPathComboxBox* _tablesDropDownList{nullptr};
  QLineEdit* _searchWhereLineEdit{nullptr};
  QComboBox* _searchCB{nullptr};

  QuickWhereClause* m_quickWhereClause{nullptr};
  FdBasedDb& mDb;

  QStringList m_studioCandidates;
  QStringList m_tagsCandidates[(int)FIELD_OP_TYPE::BUTT];
};

#endif  // MOVIEDBVIEW_H
