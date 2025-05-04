#ifndef MOVIEDBVIEW_H
#define MOVIEDBVIEW_H

#include "Component/DatabaseSearchToolBar.h"
#include "Component/MovieDatabaseMenu.h"
#include "Component/QuickWhereClause.h"

#include "Tools/FileDescriptor/FdBasedDb.h"
#include "Model/FdBasedDbModel.h"
#include "View/CustomTableView.h"

#include <QComboBox>

class MovieDBView : public CustomTableView {
 public:
  MovieDBView(FdBasedDbModel* model_,            //
              DatabaseSearchToolBar* dbSearchBar,  //
              FdBasedDb& movieDb_,               //
              QWidget* parent = nullptr);

  void subscribe();

  bool onSearchDataBase();

  bool InitMoviesTables();
  bool setCurrentMovieTable(const QString& movieTableName);

  bool onUnionTables();

  bool onInitDataBase();
  void onCreateATable();
  bool onDropATable();
  bool onDeleteFromTable();

  bool onInsertIntoTable();

  void onQuickWhereClause();

  int onCountRow();

  QString getMovieTableName() const {
    if (_tablesDropDownList == nullptr) {
      qWarning("_tablesDropDownList is nullptr");
      return "";
    }
    return _tablesDropDownList->CurrentTableName();
  }
  // should not call ~destructure after getDb() and pass to QSqlTableModel
 private:
  FdBasedDbModel* _dbModel{nullptr};
  MovieDatabaseMenu* m_movieMenu{nullptr};
  Guid2RootPathComboxBox* _tablesDropDownList{nullptr};
  QLineEdit* _searchWhereLineEdit{nullptr};
  QComboBox* _searchCB{nullptr};

  QuickWhereClause* m_quickWhereClause{nullptr};
  FdBasedDb& mDb;
};

#endif  // MOVIEDBVIEW_H
