#ifndef SCROLLBARPOLICYMENU_H
#define SCROLLBARPOLICYMENU_H

#include <QMenu>
#include "EnumIntAction.h"

extern template struct EnumIntAction<Qt::ScrollBarPolicy>;

class ScrollBarPolicyMenu : public QMenu {
  Q_OBJECT
public:
  using QMenu::QMenu;
  explicit ScrollBarPolicyMenu(const QString& name, QWidget* parent = nullptr);
  ~ScrollBarPolicyMenu();
  Qt::ScrollBarPolicy GetScrollBarPolicy() const;

signals:
  void reqScrollBarPolicyChanged(Qt::ScrollBarPolicy newScrollBarPolicy);

private:
  QString GetName() const { return title(); }
  void onActionInMenuTriggered(const QAction* pScrollBarPolicyAct);

  QAction *_AS_NEEDED{nullptr}, *_ALWAYS_ON{nullptr}, *_ALWAYS_OFF{nullptr};
  EnumIntAction<Qt::ScrollBarPolicy> mScrollBarPolicyIntAction;
};

#endif // SCROLLBARPOLICYMENU_H
