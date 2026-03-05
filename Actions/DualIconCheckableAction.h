#ifndef DUALICONCHECKABLEACTION_H
#define DUALICONCHECKABLEACTION_H

#include <QAction>
class DualIconCheckableAction : public QAction {
  Q_OBJECT
public:
  DualIconCheckableAction(const QIcon& whenUnchecked,
                          const QIcon& whenChecked,
                          const QString& whenUncheckedtext,
                          const QString& whenCheckedtext,
                          bool bIsCheckedWhenInit = false,
                          QObject* parent = nullptr);

  static DualIconCheckableAction* CreateMuteAction(QObject* parent, const bool bMute);
  static DualIconCheckableAction* CreatePauseAction(QObject* parent, const bool bPause);
  static DualIconCheckableAction* CreateFullSceenAction(QObject* parent, const bool bFullScreen);
private slots:
  void onToggled(bool checked);

private:
  void updateActionProperty(bool checked);
  const QIcon m_iconUnchecked;
  const QIcon m_iconChecked;
  const QString m_textUnchecked;
  const QString m_textChecked;
};

#endif // DUALICONCHECKABLEACTION_H
