#ifndef CUSTOMLISTVIEW_H
#define CUSTOMLISTVIEW_H

#include <QAction>
#include <QListView>
#include "AddableMenu.h"
#include "TextElideModeMenu.h"
#include "IconSizeMenu.h"

class CustomListView : public QListView {
  Q_OBJECT
public:
  explicit CustomListView(const QString& name, QWidget* parent = nullptr);
  ~CustomListView();

  void PushFrontExclusiveActions(const QList<QAction*>& acts);
  void AddItselfAction2Menu();
  void PushBackExclusiveActions(const QList<QAction*>& acts);

  void InitListView();

protected:
  void contextMenuEvent(QContextMenuEvent* event) override;
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent* event) override;

  QString m_name;
  IconSizeMenu* _ICON_SIZE_MENU{nullptr};
  TextElideModeMenu* _TEXT_ELIDE_MODE_MENU{nullptr};
  QAction* _FLOW_ORIENTATION_TTB;
  QAction* _VIEW_MODE_LIST_ICON{nullptr};
  QAction* _RESIZED_MODE_ADJUST{nullptr};
  QAction* _WRAPPING_ACTIONS{nullptr};
  QAction* _UNIFORM_ITEM_SIZES{nullptr};

  AddableMenu* m_menu {nullptr};
private:
  void SubscribePublicActions();

  void onOrientationChanged(const bool bchecked);
  void onViewModeListIconToggled(const bool bchecked);
  void onResizeModeToggled(const bool bchecked);
  void onWrapingToggled(const bool bchecked);
  void onUniformItemSizedToggled(const bool bchecked);

  void onIconScaledIndexChanged(int newScaledIndex);

  inline bool isNameExists(const QString& name) const { return LISTS_SET.contains(name); }
  static QSet<QString> LISTS_SET;
};

#endif  // CUSTOMLISTVIEW_H
