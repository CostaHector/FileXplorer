#ifndef CONTENTPANEL_H
#define CONTENTPANEL_H

#include <QStackedWidget>
#include "Component/AdvanceSearchToolBar.h"
#include "Component/CustomStatusBar.h"
#include "Component/NavigationAndAddressBar.h"
#include "Component/FolderPreview/PreviewFolder.h"

#include "MyQFileSystemModel.h"

#include "View/AdvanceSearchTableView.h"
#include "View/FileSystemListView.h"
#include "View/FileSystemTableView.h"
#include "View/FileSystemTreeView.h"
#include "View/MovieDBView.h"
#include "View/SceneTableView.h"

#include "Tools/ViewTypeTool.h"

class NavigationViewSwitcher;

class ContentPanel : public QStackedWidget {
  Q_OBJECT
 public:
  friend class NavigationViewSwitcher;

  explicit ContentPanel(PreviewFolder* previewFolder = nullptr, QWidget* parent = nullptr);
 public slots:
  bool onActionAndViewNavigate(QString newPath, bool isNewPath = true, bool isF5Force = false);
  bool onAddressToolbarPathChanged(QString newPath, bool isNewPath = true);

 public:
  auto on_searchTextChanged(const QString& targetStr) -> bool;
  auto on_searchEnterKey(const QString& targetStr) -> bool;

  auto subscribe() -> void;
  void BindNavigationAddressBar(NavigationAndAddressBar* addressBar);
  void BindDatabaseSearchToolBar(DatabaseSearchToolBar* dbSearchBar);
  void BindAdvanceSearchToolBar(AdvanceSearchToolBar* advanceSearchBar);
  void BindLogger(CustomStatusBar* logger);

  auto on_cellDoubleClicked(const QModelIndex& clickedIndex) -> bool;
  void connectSelectionChanged(ViewTypeTool::ViewType vt);
  void disconnectSelectionChanged(ViewTypeTool::ViewType vt);
  auto on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) -> bool;
  auto onAfterDirectoryLoaded(const QString& loadedPath) -> bool;

  auto keyPressEvent(QKeyEvent* e) -> void override;

  inline bool isFSView() const {
    ViewTypeTool::ViewType vt = GetCurViewType();
    return ViewTypeTool::isFSView(vt);
  }

  QModelIndex getRootIndex() const;
  inline QAbstractItemView* GetCurView() const { return dynamic_cast<QAbstractItemView*>(currentWidget()); }
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
    qCritical("Current View Type[%s] not supported", qPrintable(GetCurViewName()));
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
  DatabaseSearchToolBar* _dbSearchBar{nullptr};
  AdvanceSearchToolBar* _advanceSearchBar{nullptr};

  MyQFileSystemModel* m_fsModel;
  MyQSqlTableModel* m_dbModel{nullptr};
  AdvanceSearchModel* m_searchSrcModel{nullptr};
  SearchProxyModel* m_proxyModel{nullptr};
  ScenesTableModel* m_scenesModel{nullptr};

  FileSystemTableView* m_fsTableView{nullptr};
  FileSystemListView* m_fsListView{nullptr};
  FileSystemTreeView* m_fsTreeView{nullptr};
  MovieDBView* m_movieView{nullptr};
  AdvanceSearchTableView* m_advanceSearchView{nullptr};
  SceneTableView* m_sceneTableView{nullptr};

  PreviewFolder* _previewFolder;

  CustomStatusBar* _logger;

  QWidget* m_parent;

 private:
  QMap<ViewTypeTool::ViewType, int> m_name2ViewIndex;
};

#endif  // CONTENTPANEL_H
