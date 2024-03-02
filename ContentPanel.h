#ifndef CONTENTPANEL_H
#define CONTENTPANEL_H

#include <QStackedWidget>
#include "Component/CustomStatusBar.h"
#include "Component/FolderPreviewHTML.h"
#include "Component/FolderPreviewWidget.h"
#include "Component/NavigationAndAddressBar.h"
#include "MyQFileSystemModel.h"
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
  inline QString CurrentPath() { return m_fsm->rootPath(); }
  inline bool isFSView() const {
    auto* p = currentWidget();
    return p != nullptr and (p == m_fsView or p == m_fsListView or p == m_fsTreeView);
  }

 public slots:
  bool onActionAndViewNavigate(QString newPath, bool isNewPath = true, bool isF5Force = false);
  bool onAddressToolbarPathChanged(QString newPath, bool isNewPath = true);

 public:
  auto on_searchTextChanged(const QString& targetStr) -> bool;
  auto on_searchEnterKey(const QString& targetStr) -> bool;

  auto subscribe() -> void;
  void BindNavigationAddressBar(NavigationAndAddressBar* addressBar);
  void BindDatabaseSearchToolBar(DatabaseSearchToolBar* dbSearchBar);
  void BindCustomStatusBar(CustomStatusBar* logger);

  auto on_cellDoubleClicked(QModelIndex clickedIndex) -> bool;
  auto on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) -> bool;
  auto onAfterDirectoryLoaded(const QString& loadedPath) -> bool;

  auto keyPressEvent(QKeyEvent* e) -> void override {
    if (e->modifiers() == Qt::NoModifier and e->key() == Qt::Key_Backspace) {
      if (_addressBar) {
        _addressBar->onBackspaceEvent();
      }
      return;
    } else if (e->modifiers() == Qt::NoModifier and (e->key() == Qt::Key_Enter or e->key() == Qt::Key_Return)) {
      on_cellDoubleClicked(m_fsView->currentIndex());
      return;
    }
    QStackedWidget::keyPressEvent(e);
  }

  inline QAbstractItemView* GetView() const {
    return dynamic_cast<QAbstractItemView*>(currentWidget());
  }

  int AddView(const QString& name, QWidget* w) {
    m_name2ViewIndex[name] = addWidget(w);
    return m_name2ViewIndex[name];
  }

 public:
  struct Anchor {
    int row;
    int col;
  };
  QMap<QString, Anchor> m_anchorTags;

  NavigationAndAddressBar* _addressBar;
  DatabaseSearchToolBar* _dbSearchBar;

  MyQFileSystemModel* m_fsm;
  QMenu* m_menu;

  FileSystemTableView* m_fsView;
  FileSystemListView* m_fsListView;
  FileSystemTreeView* m_fsTreeView;

  DatabaseTableView* m_dbPanel;

  FolderPreviewHTML* previewHtml;
  FolderPreviewWidget* previewWidget;
  CustomStatusBar* _logger;

  QWidget* m_parent;

 private:
  QHash<QString, int> m_name2ViewIndex;
};

#endif  // CONTENTPANEL_H
