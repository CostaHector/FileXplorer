#ifndef VIEWSSTACKEDWIDGET_H
#define VIEWSSTACKEDWIDGET_H

#include <QStackedWidget>
#include "AdvanceSearchToolBar.h"
#include "CustomStatusBar.h"
#include "NavigationAndAddressBar.h"
#include "SelectionPreviewer.h"

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

  explicit ViewsStackedWidget(SelectionPreviewer* previewFolder = nullptr, QWidget* parent = nullptr);
 public slots:
  bool onActionAndViewNavigate(QString newPath, bool isNewPath = true, bool isF5Force = false);
  bool onAddressToolbarPathChanged(QString newPath, bool isNewPath = true);

 public:
  bool on_searchTextChanged(const QString& targetStr);
  bool on_searchEnterKey(const QString& targetStr);

  void subscribe();
  void BindNavigationAddressBar(NavigationAndAddressBar* addressBar);
  void BindDatabaseSearchToolBar(DatabaseSearchToolBar* dbSearchBar);
  void BindAdvanceSearchToolBar(AdvanceSearchToolBar* advanceSearchBar);
  void BindLogger(CustomStatusBar* logger);

  bool on_cellDoubleClicked(const QModelIndex& clickedIndex);
  void connectSelectionChanged(ViewTypeTool::ViewType vt);
  void disconnectSelectionChanged(ViewTypeTool::ViewType vt);
  bool on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  bool onAfterDirectoryLoaded(const QString& loadedPath);

  void keyPressEvent(QKeyEvent* e) override;

  inline bool isFSView() const {
    ViewTypeTool::ViewType vt = GetCurViewType();
    return ViewTypeTool::isFSView(vt);
  }

  QModelIndex getRootIndex() const;
  inline QAbstractItemView* GetCurView() const {  //
    return dynamic_cast<QAbstractItemView*>(currentWidget());
  }
  QString GetCurViewName() const;

  inline ViewTypeTool::ViewType GetCurViewType() const {
    using namespace ViewTypeTool;
    const auto* p = currentWidget();
    if (p == nullptr) {
      return ViewType::VIEW_TYPE_BUTT;
    }
    if (p == m_fsTableView) {
      return ViewType::TABLE;
    }
    if (p == m_fsListView) {
      return ViewType::LIST;
    }
    if (p == m_fsTreeView) {
      return ViewType::TREE;
    }
    if (p == m_movieView) {
      return ViewType::MOVIE;
    }
    if (p == m_advanceSearchView) {
      return ViewType::SEARCH;
    }
    if (p == m_sceneTableView) {
      return ViewType::SCENE;
    }
    if (p == m_castTableView) {
      return ViewType::CAST;
    }
    if (p == m_jsonTableView) {
      return ViewType::JSON;
    }
    qCritical("Current Index[%d] not find ViewType", currentIndex());
    return ViewType::VIEW_TYPE_BUTT;
  }

  int AddView(ViewTypeTool::ViewType vt, QWidget* w);

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

  DatabaseSearchToolBar* _dbSearchBar{nullptr};
  FdBasedDb mMovieDb;
  FdBasedDbModel* m_dbModel{nullptr};
  MovieDBView* m_movieView{nullptr};

  AdvanceSearchToolBar* _advanceSearchBar{nullptr};
  SearchProxyModel* m_searchProxyModel{nullptr};
  AdvanceSearchModel* m_searchSrcModel{nullptr};
  AdvanceSearchTableView* m_advanceSearchView{nullptr};

  ScenesListModel* m_scenesModel{nullptr};
  SceneListView* m_sceneTableView{nullptr};

  CastDBView* m_castTableView{nullptr};

  JsonProxyModel* m_jsonProxyModel{nullptr};
  JsonTableModel* m_jsonModel{nullptr};
  JsonTableView* m_jsonTableView{nullptr};

  SelectionPreviewer* _previewFolder{nullptr};

  CustomStatusBar* _logger{nullptr};

  QWidget* m_parent{nullptr};

 private:
  QMap<ViewTypeTool::ViewType, int> m_name2ViewIndex;
};

#endif  // VIEWSSTACKEDWIDGET_H
