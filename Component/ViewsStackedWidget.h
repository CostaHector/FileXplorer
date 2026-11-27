#ifndef VIEWSSTACKEDWIDGET_H
#define VIEWSSTACKEDWIDGET_H

#include <QStackedWidget>
#include "AdvanceSearchToolBar.h"
#include "NavigationAndAddressBar.h"
#include "CurrentRowPreviewer.h"

#include "AdvanceSearchTableView.h"
#include "FileSystemListView.h"
#include "FileSystemTableView.h"
#include "FileSystemTreeView.h"
#include "JsonTableView.h"
#include "MovieDBView.h"
#include "SceneListView.h"
#include "CastDBViewSplitter.h"

#include "ViewTypeTool.h"
#include "MimeDataHelper.h"
#include "SelectionUsage.h"

class ViewSwitchHelper;

class ViewsStackedWidget : public QStackedWidget {
  Q_OBJECT
 public:
  friend class ViewSwitchHelper;
  explicit ViewsStackedWidget(CurrentRowPreviewer* previewFolder = nullptr, QWidget* parent = nullptr);

 public slots:
  bool onActionAndViewNavigate(QString newPath, bool isNewPath = true);
  bool onAddressToolbarPathChanged(QString newPath, bool isNewPath = true);

 public:
  bool on_searchTextChanged(const QString& targetStr);
  bool on_searchEnterKey(const QString& targetStr);

  void subscribe();
  void BindNavigationAddressBar(NavigationAndAddressBar* addressBar);
  void BindDatabaseSearchToolBar(MovieDBSearchToolBar* dbSearchBar);
  void BindAdvanceSearchToolBar(AdvanceSearchToolBar* advanceSearchBar);
  void BindCastSearchToolBar(CastDatabaseSearchToolBar* castSearchBar);

  bool on_cellDoubleClicked(const QModelIndex& clickedIndex);
  void connectSelectionChanged(ViewTypeTool::ViewType vt);
  void disconnectSelectionChanged() {
    if (mSelectionChangedConn) {
      ViewsStackedWidget::disconnect(mSelectionChangedConn);
    }
    if (mCurrentChangedConn) {
      ViewsStackedWidget::disconnect(mCurrentChangedConn);
    }
    if (mDoubleClickedConnectConn) {
      ViewsStackedWidget::disconnect(mDoubleClickedConnectConn);
    }
  }

  void on_fsmCurrentRowChanged(const QModelIndex &current, const QModelIndex &/*previous*/);
  void on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  bool onAfterDirectoryLoaded(const QString& loadedPath);
  bool LocateToSpecifiedFile(int step);
  void RestartFileSystemWatcher();

  void keyPressEvent(QKeyEvent* e) override;

  inline bool IsCurFSView() const {
    ViewTypeTool::ViewType vt = GetVt();
    return ViewTypeTool::isFSView(vt);
  }

  std::pair<QModelIndex, QModelIndex> getTopLeftAndRightDownRectangleIndex() const;
  QModelIndex getRootIndex() const;
  const char* GetCurViewName() const;

  int AddView(ViewTypeTool::ViewType vt, QWidget* w);

  bool hasSelection() const;
  QString getRootPath() const;

  QString getFileFullPathAtIndex(const QModelIndex& proxyInd) const;
  QModelIndexList getSelectedRows() const;
  QStringList getFileNamesRelativeToRootPath() const;
  QStringList getFileFullPaths(SelectionUsage usage=SelectionUsage::DEFAULT) const;
  QStringList getModelRootPath() const;
  std::pair<QString, QStringList> getRootPathAndRel2Name(QModelIndexList& srcIndexes) const;
  MimeDataHelper::MimeDataMember getFileFullPathsAndUrls() const;
  QStringList getFullRecords() const;
  int getSelectedRowsCount() const;
  QString getCurFileFullPath() const;
  QString getCurFileName(SelectionUsage usage=SelectionUsage::DEFAULT) const;
  QFileInfo getFileInfoAtIndex(const QModelIndex& ind, SelectionUsage usage=SelectionUsage::DEFAULT) const;
  QAbstractItemView* GetCurView() const;

  /// set rating for (currently selected items)/(all items under current view root path)
  /// @param overwriteExisting: true = overwrite when rating field already exists and it's value > 0; false = skip existing rating
  /// return: affected item count
  /// return: affected item count
  int setRatingForCurrentSelection(int newRating, bool overwriteExisting=false);
  int setRatingForCurrentViewRoot(bool overwriteExisting);
  /// adjust rating (+/- delta) for (currently selected items)/(all items under current view root path)
  /// if rating field missing, treat original rating as 0
  /// return: affected item count
  int adjustRatingForCurrentSelection(int deltaRating);
  int adjustRatingForCurrentViewRoot(int deltaRating);

 public:
  struct Anchor {
    int row;
    int col;
  };
  QMap<QString, Anchor> m_anchorTags[(int)ViewTypeTool::ViewType::VIEW_TYPE_BUTT];

  NavigationAndAddressBar* _addressBar{nullptr};
  FileSystemModel* m_fsModel{nullptr};
  FileSystemTableView* m_fsTableView{nullptr};
  FileSystemListView* m_fsListView{nullptr};
  FileSystemTreeView* m_fsTreeView{nullptr};

  MovieDBSearchToolBar* _movieSearchBar{nullptr};
  FdBasedDbModel* m_movieDbModel{nullptr};
  MovieDBView* m_movieView{nullptr};

  AdvanceSearchToolBar* _advanceSearchBar{nullptr};
  SearchProxyModel* m_searchProxyModel{nullptr};
  AdvanceSearchModel* m_searchSrcModel{nullptr};
  AdvanceSearchTableView* m_advanceSearchView{nullptr};

  ScenesListModel* m_scenesModel{nullptr};
  SceneSortProxyModel* m_sceneProxyModel{nullptr};
  SceneListView* m_sceneListView{nullptr};

  CastDatabaseSearchToolBar* _castSearchBar{nullptr}; // Only for F3 to get focus
  CastDBViewSplitter* m_castDbViewSplitter{nullptr};
  CastDbModel* m_castDbModel{nullptr};
  CastDBTableView* m_castTableView{nullptr};

  QSortFilterProxyModel* m_jsonProxyModel{nullptr};
  JsonTableModel* m_jsonModel{nullptr};
  JsonTableView* m_jsonTableView{nullptr};

  CurrentRowPreviewer* _previewFolder{nullptr};

  ViewTypeTool::ViewType GetVt() const {
    return mVt;
  }
  void SetVt(ViewTypeTool::ViewType newVt) {
    mVt = newVt;
  }

 private:
  int m_name2ViewIndex[(int)ViewTypeTool::ViewType::VIEW_TYPE_BUTT] {0};
  QMetaObject::Connection mSelectionChangedConn, mCurrentChangedConn, mDoubleClickedConnectConn;
  ViewTypeTool::ViewType mVt{ViewTypeTool::DEFAULT_VIEW_TYPE};
};

#endif  // VIEWSSTACKEDWIDGET_H
