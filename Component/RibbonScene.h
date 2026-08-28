#ifndef RIBBONSCENE_H
#define RIBBONSCENE_H

#include <QToolBar>
#include "ScenePageControl.h"

class RibbonScene : public QToolBar {
public:
  explicit RibbonScene(const QString& title = "Scene Ribbons", QWidget* parent = nullptr);
  ScenePageControl* GetScenePageControl() const { return m_scenePageControl; }
private:
  ScenePageControl* m_scenePageControl{nullptr};
};

#endif // RIBBONSCENE_H
