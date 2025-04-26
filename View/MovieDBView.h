#ifndef MOVIEDBVIEW_H
#define MOVIEDBVIEW_H

#include "Component/DatabaseSearchToolBar.h"
#include "Component/MovieDatabaseMenu.h"
#include "Component/QuickWhereClause.h"

#include "Tools/FileDescriptor/MovieBaseDb.h"
#include "Model/MyQSqlTableModel.h"
#include "View/CustomTableView.h"

#include <QComboBox>

class MovieDBView : public CustomTableView {
 public:
  MovieDBView(MyQSqlTableModel* model_,            //
              DatabaseSearchToolBar* dbSearchBar,  //
              MovieBaseDb& movieDb_,               //
              QWidget* parent = nullptr);

  void subscribe();
  bool on_PlayVideo() const;

  bool onSearchDataBase(const QString& searchText) {
    _dbModel->setFilter(searchText);
    return true;
  }

  bool InitMoviesTables();
  bool setCurrentMovieTable(const QString& movieTableName);

  bool onUnionTables();

  bool onInitDataBase();
  void onCreateATable();
  bool onDropATable();
  bool onDeleteFromTable(const QString& clause = "");

  bool on_DeleteByDrive();
  bool on_DeleteByPrepath();

  bool onInsertIntoTable();

  void onQuickWhereClause();

  int onCountRow();

  QString getMovieTableName() const {
    if (_tablesDropDownList == nullptr) {
      return "";
    }
    return _tablesDropDownList->currentText();
  }
  // should not call ~destructure after getDb() and pass to QSqlTableModel
 private:
  MyQSqlTableModel* _dbModel{nullptr};
  MovieDatabaseMenu* m_movieMenu{nullptr};
  QComboBox* _tablesDropDownList{nullptr};
  QLineEdit* _searchWhereLineEdit{nullptr};
  QComboBox* _searchCB{nullptr};

  QuickWhereClause* m_quickWhereClause{nullptr};
  MovieBaseDb& mDb;
};

#endif  // MOVIEDBVIEW_H
