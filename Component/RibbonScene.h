#ifndef RIBBONSCENE_H
#define RIBBONSCENE_H

#include <QToolBar>

class RibbonScene : public QToolBar {
public:
  explicit RibbonScene(const QString& title = "Scene Ribbons", QWidget* parent = nullptr);
};

#endif // RIBBONSCENE_H
