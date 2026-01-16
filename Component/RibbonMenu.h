#ifndef RIBBONMENU_H
#define RIBBONMENU_H

#include <QTabWidget>
#include <QToolBar>
#include "ViewTypeTool.h"
#include "ScenePageControl.h"

class LogFloatingPreviewer;

class RibbonMenu : public QTabWidget {
  Q_OBJECT
 public:
  explicit RibbonMenu(QWidget *parent = nullptr);
  QToolBar* GetMenuRibbonCornerWid(QWidget* attached = nullptr);

  QToolBar* LeafFile() const;
  QToolBar* LeafHome() const;
  QToolBar* LeafView() const;
  QToolBar* LeafMovie() const;
  QToolBar* LeafCast() const;
  QToolBar* LeafScenesTools() const;
  QToolBar* LeafJson() const;
  QToolBar* LeafMediaTools() const;

  void Subscribe();
  void AfterSubscribeInitialSettings();

  void on_expandStackedWidget(const bool vis);
  void on_currentTabChangedRecordIndex(const int tabIndex);
  void whenViewTypeChanged(ViewTypeTool::ViewType vt);

  ScenePageControl* GetScenePageControlWidget() {return m_scenePageControl;}

 private:
  QAction* _EXPAND_RIBBONS{nullptr};
  QToolBar* m_corner{nullptr};

  ScenePageControl* m_scenePageControl{nullptr};

  QToolBar* m_leafFile{nullptr};
  QToolBar* m_leafHome{nullptr};
  QToolBar* m_leafView{nullptr};
  QToolBar* m_leafMovie{nullptr};
  QToolBar* m_leafCast{nullptr};
  QToolBar* m_leafScenes{nullptr};
  QToolBar* m_leafJson{nullptr};
  QToolBar* m_leafMedia{nullptr};

  int mViewType2LeafTabIndex[(int)ViewTypeTool::ViewType::VIEW_TYPE_BUTT] {0};
  static constexpr int MAX_WIDGET_HEIGHT = 400;
};

#endif  // RIBBONMENU_H
