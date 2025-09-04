#ifndef ENUMINTACTION_H
#define ENUMINTACTION_H

#include <QHash>
#include <QAction>
#include <QActionGroup>
template<typename TEMP_T>
struct EnumIntAction : public QObject {
  using QObject::QObject;
  void init(QHash<QAction*, TEMP_T> act2Enum, TEMP_T tempVal, QActionGroup::ExclusionPolicy exclusivePlcy = QActionGroup::ExclusionPolicy::None) {
    DEFAULT_ENUM = tempVal;
    mAct2Enum = act2Enum;
    mActGrp = new (std::nothrow) QActionGroup{this};
    for (auto it = act2Enum.cbegin(); it != act2Enum.cend(); ++it) {
      mVal2Enum[(int)it.value()] = it.value();
      mActGrp->addAction(it.key());
      mEnum2Act[it.value()] = it.key();
    }
    mActGrp->setExclusionPolicy(exclusivePlcy);
  }

  void setChecked(int intValue) {
    TEMP_T enumValue = val2Enum(intValue);
    mEnum2Act[enumValue]->setChecked(true);
  }

  TEMP_T val2Enum(int intVal) const {
    auto it = mVal2Enum.find(intVal);
    if (it == mVal2Enum.cend()) {
      qWarning("int[%d] not in val2Enum Hash", intVal);
      return defVal();
    }
    return it.value();
  }

  TEMP_T act2Enum(const QAction* pAct) const {
    return act2Enum(const_cast<QAction*>(pAct));
  }

  TEMP_T act2Enum(QAction* pAct) const {
    auto it = mAct2Enum.find(pAct);
    if (it == mAct2Enum.cend()) {
      return defVal();
    }
    return it.value();
  }
  TEMP_T defVal() const {
    return DEFAULT_ENUM;
  }

  QActionGroup* mActGrp{nullptr};

private:
  QHash<int, TEMP_T> mVal2Enum;
  QHash<QAction*, TEMP_T> mAct2Enum;
  QHash<TEMP_T, QAction*> mEnum2Act;
  TEMP_T DEFAULT_ENUM;
};

#endif // ENUMINTACTION_H
