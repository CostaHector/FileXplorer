#ifndef TEXTELIDEMODEMENU_H
#define TEXTELIDEMODEMENU_H

#include <QMenu>
#include "EnumIntAction.h"

extern template struct EnumIntAction<Qt::TextElideMode>;

class TextElideModeMenu : public QMenu {
  Q_OBJECT
 public:
  TextElideModeMenu(const QString& menuName, const QString& memoryName, QWidget* parent = nullptr);
  ~TextElideModeMenu();
  Qt::TextElideMode GetTextElideMode() const;

 signals:
  void reqTextElideModeChanged(Qt::TextElideMode newTextElideMode);

 private:
  QString GetName() const { return m_memoryName; }
  void onActionInMenuTriggered(const QAction* pAct);

  QAction* _TEXT_ELIDE_MODE_LEFT{nullptr};
  QAction* _TEXT_ELIDE_MODE_RIGHT{nullptr};
  QAction* _TEXT_ELIDE_MODE_MIDDLE{nullptr};
  QAction* _TEXT_ELIDE_MODE_NONE{nullptr};
  EnumIntAction<Qt::TextElideMode> mTextEditModeIntAction;
  const QString m_memoryName;
};

#endif  // TEXTELIDEMODEMENU_H
