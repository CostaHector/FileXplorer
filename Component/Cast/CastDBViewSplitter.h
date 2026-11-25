#ifndef CASTDBVIEWSPLITTER_H
#define CASTDBVIEWSPLITTER_H

#include <QSplitter>
#include "CastDbModel.h"
#include "CastBaseDb.h"
#include "CastDBTableView.h"
#include "CastDBListView.h"

class CastDatabaseSearchToolBar;
class QItemSelectionModel;

class CastDBViewSplitter : public QSplitter {
public:
  explicit CastDBViewSplitter(CastDatabaseSearchToolBar* castDbSearchBar_, QWidget* parent = nullptr);
  ~CastDBViewSplitter();

  CastDbModel* GetModel() const {
    return m_castDbModel;
  }
  CastDBTableView* GetTableView() const {
    return m_castTableView;
  }
  CastDBListView* GetListView() const {
    return m_castListView;
  }

private:
  explicit CastDBViewSplitter(const QString& castDbFullPath, const QString& castDbConnName, const QString& imgHostFullPath, CastDatabaseSearchToolBar* castDbSearchBar_, QWidget* parent = nullptr);

  void subscribe();
  void onShowCastTableView(bool bShow);
  void onShowCastListView(bool bShow);
  void onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

  bool onInitATable();
  bool onDropDeleteTable(const DbManagerHelper::DropOrDeleteE dropOrDelete);
  int onLoadFromFileSystemStructure();
  int onLoadFromPsonDirectory();

  CastDatabaseSearchToolBar* _castDbSearchBar;

  CastBaseDb* m_castDb{nullptr};
  CastDbModel* m_castDbModel{nullptr};
  QItemSelectionModel* m_sharedSelectionModel{nullptr};
  CastDBTableView* m_castTableView{nullptr};
  CastDBListView* m_castListView{nullptr};
};

#endif // CASTDBVIEWSPLITTER_H
