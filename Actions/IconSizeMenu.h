#ifndef ICONSIZEMENU_H
#define ICONSIZEMENU_H
#include <QObject>
#include <QMenu>

class IconSizeMenu : public QMenu {
  Q_OBJECT
public:
  IconSizeMenu(const QString& menuName, const QString& belongedToName, QWidget* parent=nullptr);
  ~IconSizeMenu();
  QString GetName() const { return mBelongToName; }
  int GetScaledIndex() const { return mIconSizeScaledIndex; }
  bool UpdateScaledIndexInWheelEvent(int newScaledSize);

signals:
  void iconScaledIndexChanged(const int &scaledIndex);

private:
  bool EmitIconScaledIndexChanged(const QAction* pAct);

  QActionGroup* mSizeActGroup{nullptr};
  QList<QAction*> mSizeActsList;
  const QString mBelongToName;
  int mIconSizeScaledIndex{10};
};

#endif // ICONSIZEMENU_H
