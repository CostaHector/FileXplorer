#include "SizeChangeAnimation.h"
#include <QWidget>

SizeChangeAnimation::SizeChangeAnimation(Qt::Orientation ori, int expandVal, int noExpandValue, bool bAnimationEnabled)
  : QObject{nullptr}
  , mOri{ori}
  , mExpandVal{expandVal}
  , mNoExpandValue{noExpandValue}
  , mAnimationEnabled{bAnimationEnabled} {}

void SizeChangeAnimation::doPreOperation() {
  if (mFuncBeforeStart) {
    mFuncBeforeStart();
  }
  mFuncBeforeStart = nullptr;
}

void SizeChangeAnimation::doPostOperation() {
  if (mFuncWhenFinished) {
    mFuncWhenFinished();
  }
  mFuncWhenFinished = nullptr;
}

void SizeChangeAnimation::registerCallbackBeforeStart(CallbackFunc callbackBeforeStart) {
  if (!callbackBeforeStart) { // no callback
    return;
  }
  if (mFuncBeforeStart) { // cannot register again
    return;
  }
  mFuncBeforeStart = callbackBeforeStart;
}

void SizeChangeAnimation::registerCallbackWhenFinished(CallbackFunc callbackWhenFinished) {
  if (!callbackWhenFinished) { // no callback
    return;
  }
  if (mFuncWhenFinished) { // cannot register again
    return;
  }
  mFuncWhenFinished = callbackWhenFinished;
}

void SizeChangeAnimation::operator()(QWidget *pWid, int currentVal, bool bExpand) {
  doPreOperation();
  std::function<void(int)> fallbackCallback;
  QByteArray propertyName;
  if (mOri == Qt::Orientation::Vertical) {
    fallbackCallback = std::bind(&QWidget::setMaximumWidth, pWid, std::placeholders::_1);
    propertyName = "maximumWidth";
  } else {
    fallbackCallback = std::bind(&QWidget::setMaximumHeight, pWid, std::placeholders::_1);
    propertyName = "maximumHeight";
  }
  if (!mAnimationEnabled) {
    fallbackCallback(bExpand ? mExpandVal : mNoExpandValue);
    doPostOperation();
    return;
  }
  QPropertyAnimation *myAnimation = new (std::nothrow) QPropertyAnimation{pWid, propertyName, pWid};
  if (myAnimation == nullptr) {
    fallbackCallback(bExpand ? mExpandVal : mNoExpandValue);
    doPostOperation();
    return;
  }
  myAnimation->setDuration(200);
  myAnimation->setStartValue(currentVal);
  myAnimation->setEndValue(bExpand ? mExpandVal : mNoExpandValue);
  if (mFuncWhenFinished) {
    QObject::connect(myAnimation, &QPropertyAnimation::finished, this, &SizeChangeAnimation::doPostOperation);
  }
  myAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}
