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
  bool isVerticalLongerMode() const { return mIconVerticalLongerMode; }
  bool UpdateScaledIndexInWheelEvent(int newScaledSize);

signals:
  void iconScaledIndexChanged(const int &scaledIndex);
  void iconVerticalLongerModeChanged(bool bIconVerticalLongerMode);

private:
  bool EmitIconScaledIndexChanged(const QAction* pAct);
  bool EmitIconVerticalLongerModeChanged(bool bIconVerticalLongerMode);

  QAction* mIconVerticalLongerModeAct{nullptr};
  QActionGroup* mSizeActGroup{nullptr};
  QList<QAction*> mSizeActsList;
  const QString mBelongToName;
  bool mIconVerticalLongerMode{false};
  int mIconSizeScaledIndex{10};
};

#endif // ICONSIZEMENU_H
