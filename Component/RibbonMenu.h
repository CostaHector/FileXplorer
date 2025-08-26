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
  QToolBar* LeafMovie() const;
  QToolBar* LeafCast() const;
  QToolBar* LeafJson() const;
  QToolBar* LeafScenesTools() const;
  QToolBar* LeafMediaTools() const;

  void Subscribe();

  void on_officeStyleWidgetVisibilityChanged(const bool vis);
  void on_currentTabChangedRecordIndex(const int tabIndex);

 private:
  QToolBar* m_corner{nullptr};

  QToolBar* m_leafFile{nullptr};
  QToolBar* m_leafHome{nullptr};
  QToolBar* m_leafView{nullptr};
  QToolBar* m_leafMovie{nullptr};
  QToolBar* m_leafCast{nullptr};
  QToolBar* m_leafJson{nullptr};
  QToolBar* m_leafScenes{nullptr};
  QToolBar* m_leafMedia{nullptr};

  static constexpr int MAX_WIDGET_HEIGHT = 400;
};

#endif  // RIBBONMENU_H
