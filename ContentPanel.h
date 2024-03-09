#ifndef CONTENTPANEL_H
#define CONTENTPANEL_H

#include <QStackedWidget>
#include "Component/AdvanceSearchToolBar.h"
#include "Component/CustomStatusBar.h"
#include "Component/FolderPreviewHTML.h"
#include "Component/FolderPreviewWidget.h"
#include "Component/NavigationAndAddressBar.h"

#include "MyQFileSystemModel.h"

#include "View/AdvanceSearchTableView.h"
#include "View/DatabaseTableView.h"
#include "View/FileSystemListView.h"
#include "View/FileSystemTableView.h"
#include "View/FileSystemTreeView.h"

class NavigationViewSwitcher;

class ContentPanel : public QStackedWidget {
  Q_OBJECT
 public:
  friend class NavigationViewSwitcher;

  explicit ContentPanel(FolderPreviewHTML* previewHtml_ = nullptr, FolderPreviewWidget* previewWidget_ = nullptr, QWidget* parent = nullptr);
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
  auto on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) -> bool;
  auto onAfterDirectoryLoaded(const QString& loadedPath) -> bool;

  auto keyPressEvent(QKeyEvent* e) -> void override;

  inline bool isFSView() const {
    const auto* p = currentWidget();
    return p != nullptr and (p == m_fsTableView or p == m_fsListView or p == m_fsTreeView);
  }
  inline QAbstractItemView* GetCurView() const {
    return dynamic_cast<QAbstractItemView*>(currentWidget());
  }
  QAbstractItemView* GetView(const QString& name) const;
  QString GetCurViewName() const;
  int AddView(const QString& viewType, QWidget* w);


  QString getRootPath() const;
  QString getFilePath(const QModelIndex& ind) const;
  QModelIndexList getSelectedRows() const;
  QStringList getFileNames() const;
  QStringList getFilePaths() const;
  QStringList getFilePrepaths() const;
  QStringList getTheJpgFolderPaths() const;
  std::pair<QStringList, QList<QUrl>> getFilePathsAndUrls(const Qt::DropAction dropAct = Qt::IgnoreAction) const;

  int getSelectedRowsCount() const;
  QFileInfo getFileInfo(const QModelIndex& ind) const;

 public:
  struct Anchor {
    int row;
    int col;
  };
  QMap<QString, Anchor> m_anchorTags;

  NavigationAndAddressBar* _addressBar;
  DatabaseSearchToolBar* _dbSearchBar;
  AdvanceSearchToolBar* _advanceSearchBar;

  MyQFileSystemModel* m_fsModel;
  MyQSqlTableModel* m_dbModel{nullptr};
  AdvanceSearchModel* m_srcModel{nullptr};
  SearchProxyModel* m_proxyModel{nullptr};

  QMenu* m_menu;

  FileSystemTableView* m_fsTableView{nullptr};
  FileSystemListView* m_fsListView{nullptr};
  FileSystemTreeView* m_fsTreeView{nullptr};
  DatabaseTableView* m_dbPanel{nullptr};
  AdvanceSearchTableView* m_advanceSearchView{nullptr};

  FolderPreviewHTML* previewHtml;
  FolderPreviewWidget* previewWidget;
  CustomStatusBar* _logger;

  QWidget* m_parent;

 private:
  QHash<QString, int> m_name2ViewIndex;
};

#endif  // CONTENTPANEL_H
