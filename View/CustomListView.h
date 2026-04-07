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
  explicit CustomListView(const QString& instName, QWidget* parent = nullptr);
  ~CustomListView();
  const QString& GetName() const { return m_name; }

  void PushFrontExclusiveActions(const QList<QAction*>& acts);
  void PushBackExclusiveActions(const QList<QAction*>& acts);

  void InitListView();

 protected:
  void contextMenuEvent(QContextMenuEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  virtual void initExclusivePreferenceSetting() {}

  const QString m_name;
  bool m_defaultFlowLeft2Right{false}, m_defaultViewModeIcon{false}, m_defaultWrapping{false};

  IconSizeMenu* _ICON_SIZE_MENU{nullptr};
  TextElideModeMenu* _TEXT_ELIDE_MODE_MENU{nullptr};
  QAction* _FLOW_ORIENTATION_LTR{nullptr};
  QAction* _VIEW_MODE_LIST_ICON{nullptr};
  QAction* _RESIZED_MODE_ADJUST{nullptr};
  QAction* _WRAPPING_ACTIONS{nullptr};
  QAction* _UNIFORM_ITEM_SIZES{nullptr};

 private:
  void SubscribePublicActions();
  void AddItselfAction2Menu();

  void onFlowOrientationChanged(const bool bLeft2Right);
  void onViewModeListIconToggled(const bool bIconMode);
  void onResizeModeToggled(const bool bAdjust);
  void onWrapingToggled(const bool bWapping);
  void onUniformItemSizedToggled(const bool bUniform);

  void onIconScaledIndexChanged(int newScaledIndex);

  AddableMenu* m_menu{nullptr};

  inline bool isNameExists(const QString& name) const { return mListInstSet.contains(name); }
  static QSet<QString> mListInstSet;
};

#endif  // CUSTOMLISTVIEW_H
