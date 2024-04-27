#ifndef RIBBONMENU_H
#define RIBBONMENU_H

#include <QTabWidget>
#include <QToolBar>

class RibbonMenu : public QTabWidget {
 public:
  explicit RibbonMenu(QWidget *parent = nullptr);
  QToolBar* GetMenuRibbonCornerWid(QWidget* attached = nullptr);

  QToolBar* LeafFile() const;
  QToolBar* LeafHome() const;
  QToolBar* LeafView() const;
  QToolBar* LeafDatabase() const;
  QToolBar* LeafMediaTools() const;

  void Subscribe();

  void on_officeStyleWidgetVisibilityChanged(const bool vis);;
  void on_currentTabChangedRecordIndex(const int tabIndex);

 private:
  QToolBar* m_corner;

  QToolBar* m_leafFile;
  QToolBar* m_leafHome;
  QToolBar* m_leafView;
  QToolBar* m_leafDatabase;

  QToolBar* m_leafMore;

  static constexpr int MAX_WIDGET_HEIGHT = 400;
};

#endif  // RIBBONMENU_H
