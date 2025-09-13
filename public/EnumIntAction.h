#ifndef ENUMINTACTION_H
#define ENUMINTACTION_H

#include <QHash>
#include <QMap>
#include <QAction>
#include <QActionGroup>
#include "Logger.h"

template<typename ENUM_T>
struct EnumIntAction : public QObject {
public:
  using QObject::QObject;
  operator bool() const {
    return mActGrp != nullptr && !mVal2Enum.isEmpty();
  }

  void init(QHash<QAction*, ENUM_T> act2Enum, ENUM_T tempVal, QActionGroup::ExclusionPolicy exclusivePlcy = QActionGroup::ExclusionPolicy::None) {
    DEFAULT_ENUM = tempVal;
    mAct2Enum = act2Enum;

    for (auto it = act2Enum.cbegin(); it != act2Enum.cend(); ++it) {
      mVal2Enum[(int)it.value()] = it.value();
      mEnum2Act[it.value()] = it.key();
    }

    mActGrp = new (std::nothrow) QActionGroup{this};
    for (auto it = mEnum2Act.cbegin(); it != mEnum2Act.cend(); ++it) {
      mActGrp->addAction(it.value());
    }
    mActGrp->setExclusionPolicy(exclusivePlcy);

    mActionsInNameAscending = mActionsInEnumAscending = mActGrp->actions();
    std::sort(mActionsInNameAscending.begin(), mActionsInNameAscending.end(), [](const QAction* left, const QAction* right)->bool{
      return left != nullptr && right != nullptr && left->text() < right->text();
    });
  }

  QAction* setCheckedIfActionExist(ENUM_T enumValue) {
    // when QActionGroup::ExclusionPolicy::ExclusiveOptional,
    // intValue -> defVal() may have no action correspond
    auto it = mEnum2Act.find(enumValue);
    if (it == mEnum2Act.cend()) {
      LOG_D("enumValue:%d have no action", static_cast<int>(enumValue));
      return nullptr;
    }
    mEnum2Act[enumValue]->setChecked(true);
    return mEnum2Act[enumValue];
  }

  QAction* setCheckedIfActionExist(int intValue) {
    ENUM_T enumValue = intVal2Enum(intValue);
    return setCheckedIfActionExist(enumValue);
  }

  ENUM_T intVal2Enum(int intVal) const {
    auto it = mVal2Enum.find(intVal);
    if (it == mVal2Enum.cend()) {
      LOG_W("int[%d] not in intVal2Enum Hash", intVal);
      return defVal();
    }
    return it.value();
  }

  ENUM_T act2Enum(const QAction* pAct) const {
    return act2Enum(const_cast<QAction*>(pAct));
  }

  ENUM_T act2Enum(QAction* pAct) const {
    auto it = mAct2Enum.find(pAct);
    if (it == mAct2Enum.cend()) {
      return defVal();
    }
    return it.value();
  }
  ENUM_T defVal() const {
    return DEFAULT_ENUM;
  }
  ENUM_T curVal() const {
    if (mActGrp == nullptr || mActGrp->exclusionPolicy() == QActionGroup::ExclusionPolicy::None) {
      LOG_W("mActGrp is nullptr or ExclusionPolicy=None");
      return DEFAULT_ENUM;
    }
    return act2Enum(mActGrp->checkedAction());
  }
  QActionGroup* getActionGroup() const { // not only manual click(from user interact), but also emit
    return mActGrp;
  }
  const QList<QAction*>& getActionEnumAscendingList() const {
    return mActionsInEnumAscending;
  }
  const QList<QAction*>& getActionNameAscendingList() const {
    return mActionsInNameAscending;
  }

private:
  QActionGroup* mActGrp{nullptr};
  QHash<int, ENUM_T> mVal2Enum;
  QHash<QAction*, ENUM_T> mAct2Enum;
  QMap<ENUM_T, QAction*> mEnum2Act;
  ENUM_T DEFAULT_ENUM;
  QList<QAction*> mActionsInEnumAscending;
  QList<QAction*> mActionsInNameAscending;
};

#endif // ENUMINTACTION_H
