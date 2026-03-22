#include "IconSizeMenu.h"
#include "ImageTool.h"
#include "PublicMacro.h"

IconSizeMenu::IconSizeMenu(const QString& menuName, const QString& belongedToName, QWidget* parent) //
  : QMenu{menuName, parent}
  , mBelongToName{belongedToName} {
  CHECK_NULLPTR_RETURN_VOID(parent);
  CHECK_FALSE_RETURN_VOID(!mBelongToName.isEmpty());

  mSizeActGroup = new QActionGroup{this};
  mSizeActGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  mIconSizeScaledIndex = IMAGE_SIZE::GetInitialScaledSize(GetName());

  for (int i = 0; i < IMAGE_SIZE::ICON_SIZE_CANDIDATES_N; ++i) {
    const QString actName = IMAGE_SIZE::HumanReadFriendlySize(i);
    QAction* pSizeAct = new QAction(actName, this);
    pSizeAct->setData(i);
    pSizeAct->setCheckable(true);
    if (i == mIconSizeScaledIndex) {
      pSizeAct->setChecked(true);
    }
    mSizeActGroup->addAction(pSizeAct);
    addAction(pSizeAct);
    // not the first and the last element, every five element add a separator
    if ((i + 1) % 5 == 0 && i + 1 != IMAGE_SIZE::ICON_SIZE_CANDIDATES_N) {
      addSeparator();
    }
  }
  setIcon(QIcon{":img/ICON_SIZE"});
  setToolTipsVisible(true);

  mSizeActsList = mSizeActGroup->actions();
  connect(this, &QMenu::triggered, this, &IconSizeMenu::EmitIconScaledIndexChanged);
}

IconSizeMenu::~IconSizeMenu() {
  IMAGE_SIZE::SaveInitialScaledSize(GetName(), mIconSizeScaledIndex);
}

bool IconSizeMenu::EmitIconScaledIndexChanged(const QAction* pAct) {
  bool bOk = false;
  int newScaledSize = pAct->data().toInt(&bOk);
  if (!bOk) {
    LOG_W("data property in QAction is not a number");
    return false;
  }
  newScaledSize = IMAGE_SIZE::clampScaledIndex(newScaledSize);
  if (mIconSizeScaledIndex == newScaledSize) {
    return false;
  }
  mIconSizeScaledIndex = newScaledSize;
  emit iconScaledIndexChanged(mIconSizeScaledIndex);
  return true;
}

bool IconSizeMenu::UpdateScaledIndexInWheelEvent(int newScaledSize) {
  newScaledSize = IMAGE_SIZE::clampScaledIndex(newScaledSize);
  if (newScaledSize == mIconSizeScaledIndex) {
    return false;
  }
  mIconSizeScaledIndex = newScaledSize;
  mSizeActsList[newScaledSize]->setChecked(true);
  return true;
}
