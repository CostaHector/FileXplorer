#ifndef SIZECHANGEANIMATION_H
#define SIZECHANGEANIMATION_H

#include <QPropertyAnimation>
struct SizeChangeAnimation : public QObject {
  Q_OBJECT
public:
  using CallbackFunc = std::function<void(void)>;
  SizeChangeAnimation(Qt::Orientation ori, int expandVal, int noExpandValue, bool bAnimationEnabled);

  void registerCallbackBeforeStart(CallbackFunc callbackBeforeStart);
  void registerCallbackWhenFinished(CallbackFunc callbackWhenFinished);
  void operator()(QWidget* pWid, int currentVal, bool bExpand);

private:
  void doPreOperation();
  void doPostOperation();

  Qt::Orientation mOri{Qt::Orientation::Vertical};
  int mExpandVal{0}, mNoExpandValue{0};
  bool mAnimationEnabled{false};
  CallbackFunc mFuncBeforeStart, mFuncWhenFinished;
};
#endif // SIZECHANGEANIMATION_H
