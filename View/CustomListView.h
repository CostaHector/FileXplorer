#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include <QAction>
#include <QListView>
#include <QMenu>
#include "EnumIntAction.h"

extern template struct EnumIntAction<Qt::TextElideMode>;

class CustomListView : public QListView {
  Q_OBJECT
public:
  explicit CustomListView(const QString& name, QWidget* parent = nullptr);
  ~CustomListView();
  void contextMenuEvent(QContextMenuEvent* event) override;
  void wheelEvent(QWheelEvent *event) override;

  void BindMenu(QMenu* menu);

  void InitListView();
  void mousePressEvent(QMouseEvent* event) override;

  int GetCurImageSizeScale() const {return mCurIconSizeIndex;}
  bool setIconSizeScaledIndex(int newScaledIndex);
protected:
  QString m_name;

  QAction* _TEXT_ELIDE_MODE_LEFT{nullptr};
  QAction* _TEXT_ELIDE_MODE_RIGHT{nullptr};
  QAction* _TEXT_ELIDE_MODE_MIDDLE{nullptr};
  QAction* _TEXT_ELIDE_MODE_NONE{nullptr};
  EnumIntAction<Qt::TextElideMode> mTextEditModeIntAction;
  QMenu* _TEXT_ELIDE_MODE_MENU{nullptr};

  QAction* _FLOW_ORIENTATION;
  QAction* _VIEW_MODE_LIST_ICON{nullptr};
  QAction* _RESIZED_MODE_FIXED_OR_ADJUST{nullptr};
  QAction* _WRAPING_ACTIONS{nullptr};
  QAction* _UNIFORM_ITEM_SIZES{nullptr};

private:
  void SubscribePublicActions();

  void onTextElideModeChanged(const QAction* pAct);
  void onOrientationChanged(const bool bchecked);
  void onViewModeListIconToggled(const bool bchecked);
  void onResizeModeToggled(const bool bchecked);
  void onWrapingToggled(const bool bchecked);
  void onUniformItemSizedToggled(const bool bchecked);

  QMenu* m_menu {nullptr};

  inline bool isNameExists(const QString& name) const { return LISTS_SET.contains(name); }
  static QSet<QString> LISTS_SET;
  int mCurIconSizeIndex = 1;
};

#endif  // CUSTOMLISTVIEW_H
