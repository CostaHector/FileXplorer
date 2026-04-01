#ifndef SCROLLBARPOLICYMENU_H
#define SCROLLBARPOLICYMENU_H

#include <QMenu>
#include "EnumIntAction.h"

extern template struct EnumIntAction<Qt::ScrollBarPolicy>;

class ScrollBarPolicyMenu : public QMenu {
  Q_OBJECT
public:
  ScrollBarPolicyMenu(const QString& menuName, const QString& belongToName, QWidget* parent = nullptr);
  ~ScrollBarPolicyMenu();
  Qt::ScrollBarPolicy GetScrollBarPolicy() const;
  QString GetName() const { return m_memoryName; }

signals:
  void reqScrollBarPolicyChanged(Qt::ScrollBarPolicy newScrollBarPolicy);

private:
  void onActionInMenuTriggered(const QAction* pScrollBarPolicyAct);

  QAction *_AS_NEEDED{nullptr}, *_ALWAYS_ON{nullptr}, *_ALWAYS_OFF{nullptr};
  EnumIntAction<Qt::ScrollBarPolicy> mScrollBarPolicyIntAction;
  const QString m_memoryName;
};

#endif // SCROLLBARPOLICYMENU_H
