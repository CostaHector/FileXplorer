#ifndef VIEWSSTACKEDWIDGET_H
#define VIEWSSTACKEDWIDGET_H

#include <QStackedWidget>
#include "AdvanceSearchToolBar.h"
#include "CustomStatusBar.h"
#include "NavigationAndAddressBar.h"
#include "CurrentRowPreviewer.h"

#include "AdvanceSearchTableView.h"
#include "FileSystemListView.h"
#include "FileSystemTableView.h"
#include "FileSystemTreeView.h"
#include "JsonTableView.h"
#include "MovieDBView.h"
#include "SceneListView.h"
#include "CastDBView.h"

#include "ViewTypeTool.h"
#include "FdBasedDb.h"


class ToolBarAndViewSwitcher;

class ViewsStackedWidget : public QStackedWidget {
  Q_OBJECT
 public:
  friend class ToolBarAndViewSwitcher;
  explicit ViewsStackedWidget(CurrentRowPreviewer* previewFolder = nullptr, QWidget* parent = nullptr);

 public slots:
  bool onActionAndViewNavigate(QString newPath, bool isNewPath = true, bool isF5Force = false);
  bool onAddressToolbarPathChanged(QString newPath, bool isNewPath = true);

 public:
  bool on_searchTextChanged(const QString& targetStr);
  bool on_searchEnterKey(const QString& targetStr);

  void subscribe();
  void BindNavigationAddressBar(NavigationAndAddressBar* addressBar);
  void BindDatabaseSearchToolBar(MovieDBSearchToolBar* dbSearchBar);
  void BindAdvanceSearchToolBar(AdvanceSearchToolBar* advanceSearchBar);
  void BindCastSearchToolBar(CastDatabaseSearchToolBar* castSearchBar);
  void BindLogger(CustomStatusBar* logger);

  bool on_cellDoubleClicked(const QModelIndex& clickedIndex);
  void connectSelectionChanged(ViewTypeTool::ViewType vt);
  void disconnectSelectionChanged() {
    if (mSelectionChangedConn) {
      ViewsStackedWidget::disconnect(mSelectionChangedConn);
    }
    if (mCurrentChangedConn) {
      ViewsStackedWidget::disconnect(mCurrentChangedConn);
    }
  }
  void on_currentRowChanged(const QModelIndex &current, const QModelIndex &/*previous*/);
  void on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  bool onAfterDirectoryLoaded(const QString& loadedPath);

  void keyPressEvent(QKeyEvent* e) override;

  inline bool IsCurFSView() const {
    ViewTypeTool::ViewType vt = GetVt();
    return ViewTypeTool::isFSView(vt);
  }

  std::pair<QModelIndex, QModelIndex> getTopLeftAndRightDownRectangleIndex() const;
  QModelIndex getRootIndex() const;
  inline QAbstractItemView* GetCurView() const {  //
    return dynamic_cast<QAbstractItemView*>(currentWidget());
  }
  const char* GetCurViewName() const;

  int AddView(ViewTypeTool::ViewType vt, QWidget* w);

  bool hasSelection() const;
  QString getRootPath() const;
  QString getFilePath(const QModelIndex& ind) const;
  QModelIndexList getSelectedRows() const;
  QStringList getFileNames() const;
  QStringList getFilePaths() const;
  QStringList getFilePrepaths() const;
  QStringList getTheJpgFolderPaths() const;
  QStringList getFullRecords() const;

  std::pair<QStringList, QList<QUrl>> getFilePathsAndUrls(const Qt::DropAction dropAct = Qt::IgnoreAction) const;
  std::pair<QStringList, QStringList> getFilePrepathsAndName(const bool isSearchRecycle = false) const;

  int getSelectedRowsCount() const;
  QString getCurFilePath() const;
  QString getCurFileName() const;
  QFileInfo getFileInfo(const QModelIndex& ind) const;

 public:
  struct Anchor {
    int row;
    int col;
  };
  QMap<QString, Anchor> m_anchorTags;

  NavigationAndAddressBar* _addressBar{nullptr};
  FileSystemModel* m_fsModel{nullptr};
  FileSystemTableView* m_fsTableView{nullptr};
  FileSystemListView* m_fsListView{nullptr};
  FileSystemTreeView* m_fsTreeView{nullptr};

  MovieDBSearchToolBar* _movieSearchBar{nullptr};
  FdBasedDb mMovieDb;
  FdBasedDbModel* m_movieDbModel{nullptr};
  MovieDBView* m_movieView{nullptr};

  AdvanceSearchToolBar* _advanceSearchBar{nullptr};
  SearchProxyModel* m_searchProxyModel{nullptr};
  AdvanceSearchModel* m_searchSrcModel{nullptr};
  AdvanceSearchTableView* m_advanceSearchView{nullptr};

  ScenesListModel* m_scenesModel{nullptr};
  SceneListView* m_sceneTableView{nullptr};

  CastDatabaseSearchToolBar* _castSearchBar{nullptr}; // Only for F3 to get focus
  CastBaseDb mCastDb;
  CastDbModel* m_castDbModel{nullptr};
  CastDBView* m_castTableView{nullptr};

  JsonProxyModel* m_jsonProxyModel{nullptr};
  JsonTableModel* m_jsonModel{nullptr};
  JsonTableView* m_jsonTableView{nullptr};

  CurrentRowPreviewer* _previewFolder{nullptr};

  CustomStatusBar* _logger{nullptr};

  QWidget* m_parent{nullptr};

  ViewTypeTool::ViewType GetVt() const {
    return mVt;
  }
  void SetVt(ViewTypeTool::ViewType newVt) {
    mVt = newVt;
  }

 private:
  QMap<ViewTypeTool::ViewType, int> m_name2ViewIndex;
  QMetaObject::Connection mSelectionChangedConn, mCurrentChangedConn;
  ViewTypeTool::ViewType mVt{ViewTypeTool::ViewType::TABLE};
};

#endif  // VIEWSSTACKEDWIDGET_H
