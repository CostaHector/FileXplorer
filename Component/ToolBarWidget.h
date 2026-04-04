#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QBoxLayout>
#include <QAction>
#include <QLabel>
#include <unordered_set>

class ToolBarWidget : public QWidget {
 public:
  explicit ToolBarWidget(QBoxLayout::Direction direction = QBoxLayout::Direction::LeftToRight, QWidget* parent = nullptr);
  bool setDirection(QBoxLayout::Direction direction);
  Qt::Orientation orientation() const {
    switch (mLayout->direction()) {
      case QBoxLayout::Direction::LeftToRight:
      case QBoxLayout::Direction::RightToLeft:
        return Qt::Orientation::Horizontal;
      case QBoxLayout::Direction::TopToBottom:
      case QBoxLayout::Direction::BottomToTop:
      default:
        return Qt::Orientation::Vertical;
    }
  }

  QList<QAction*> actions() const { return mActions; }

  int setToolButtonSizePolicy(const QSizePolicy& newSizePolicy, bool bRecursive = false);
  QSizePolicy ToolButtonSizePolicy() const { return mToolButtonSizePolicy; }

  int setIconSize(const QSize& newIconSize, bool bRecursive = false);
  QSize iconSize() const { return mIconSize; }

  int setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle, bool bRecursive = false);
  Qt::ToolButtonStyle toolButtonStyle() const { return mToolButtonStyle; }

  void addWidget(QWidget* parent, int stretch = 0);
  QAction* addAction(const QIcon& icon, const QString& text);
  bool addAction(QAction* act);
  void addSpacing(int size = 1);
  void addStretch(int stretch = 0);
  QLabel* addString(const QString& sep = "|");
  QFrame* addSeparator();

  QToolButton* createToolButton(QAction* act,
                                bool bToolButtonStyleFixed = DEF_TOOL_BUTTON_STYLE_FIXED);

 private:
  QBoxLayout* mLayout{nullptr};
  std::unordered_set<QString> mToolButtonStyleFixedObjNameSet;

  QList<QAction*> mActions;

  QSizePolicy mToolButtonSizePolicy{QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed};
  Qt::ToolButtonStyle mToolButtonStyle{DEF_TOOL_BUTTON_STYLE};
  QSize mIconSize{-1, -1};
  static constexpr Qt::ToolButtonStyle DEF_TOOL_BUTTON_STYLE = Qt::ToolButtonStyle::ToolButtonIconOnly;
  static constexpr bool DEF_TOOL_BUTTON_STYLE_FIXED = false;
};
#endif  // TOOLBARWIDGET_H
