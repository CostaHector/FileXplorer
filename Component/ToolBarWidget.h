#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QBoxLayout>
#include <QAction>
#include <unordered_set>

class ToolBarWidget : public QWidget {
public:
  explicit ToolBarWidget(QBoxLayout::Direction direction = QBoxLayout::Direction::LeftToRight, QWidget *parent = nullptr);
  void setDirection(QBoxLayout::Direction direction);
  int setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle, bool bRecursive = false);
  void addWidget(QWidget *parent, int stretch = 0);
  void addSpacing(int size = 1);
  void addStretch(int stretch = 0);
  void addString(const QString &sep = "|");
  void addSeparator();

  QToolButton *createToolButton(const QIcon &icon,
                                const QString &text,
                                const QString &toolTip = "",
                                bool checkable = false,
                                Qt::ToolButtonStyle toolButtonStyle = Qt::ToolButtonStyle::ToolButtonIconOnly,
                                bool bToolButtonStyleFixed = false);

  QToolButton *createToolButton(QAction *act,
                                Qt::ToolButtonStyle toolButtonStyle = Qt::ToolButtonStyle::ToolButtonIconOnly,
                                bool bToolButtonStyleFixed = false);

private:
  QBoxLayout *mLayout{nullptr};
  std::unordered_set<QString> mToolButtonStyleFixedObjNameSet;
};
#endif // TOOLBARWIDGET_H
