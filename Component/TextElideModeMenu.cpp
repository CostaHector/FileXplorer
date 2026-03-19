#include "TextElideModeMenu.h"
#include "MemoryKey.h"
#include "PublicMacro.h"

TextElideModeMenu::TextElideModeMenu(const QString& menuName, const QString& memoryName, QWidget* parent)
    : QMenu{menuName, parent}, m_memoryName{memoryName} {
  CHECK_NULLPTR_RETURN_VOID(parent);
  CHECK_FALSE_RETURN_VOID(!memoryName.isEmpty());

  setIcon(QIcon{":img/LIST_TEXT_ELIDE_MODE"});

  _TEXT_ELIDE_MODE_LEFT = addAction(QIcon{":img/LIST_TEXT_ELIDE_MODE_LEFT"}, tr("Elide Left"));
  CHECK_FALSE_RETURN_VOID(_TEXT_ELIDE_MODE_LEFT);
  _TEXT_ELIDE_MODE_LEFT->setCheckable(true);

  _TEXT_ELIDE_MODE_RIGHT = addAction(QIcon{":img/LIST_TEXT_ELIDE_MODE_RIGHT"}, tr("Elide Right"));
  CHECK_FALSE_RETURN_VOID(_TEXT_ELIDE_MODE_RIGHT);
  _TEXT_ELIDE_MODE_RIGHT->setCheckable(true);

  _TEXT_ELIDE_MODE_MIDDLE = addAction(QIcon{":img/LIST_TEXT_ELIDE_MODE_MIDDLE"}, tr("Elide Middle"));
  CHECK_FALSE_RETURN_VOID(_TEXT_ELIDE_MODE_MIDDLE);
  _TEXT_ELIDE_MODE_MIDDLE->setCheckable(true);

  _TEXT_ELIDE_MODE_NONE = addAction(QIcon{":img/LIST_TEXT_ELIDE_MODE"}, tr("Elide None"));
  CHECK_FALSE_RETURN_VOID(_TEXT_ELIDE_MODE_NONE);
  _TEXT_ELIDE_MODE_NONE->setCheckable(true);

  mTextEditModeIntAction.init(
      {
       {_TEXT_ELIDE_MODE_LEFT, Qt::TextElideMode::ElideLeft},      //
       {_TEXT_ELIDE_MODE_RIGHT, Qt::TextElideMode::ElideRight},    //
       {_TEXT_ELIDE_MODE_MIDDLE, Qt::TextElideMode::ElideMiddle},  //
       {_TEXT_ELIDE_MODE_NONE, Qt::TextElideMode::ElideNone},
       },  //
      Qt::TextElideMode::ElideNone, QActionGroup::ExclusionPolicy::Exclusive);
  int elideInt = Configuration().value(GetName(), (int)mTextEditModeIntAction.defVal()).toInt();
  mTextEditModeIntAction.setCheckedIfActionExist(elideInt);

  connect(this, &QMenu::triggered, this, &TextElideModeMenu::onActionInMenuTriggered);
}

TextElideModeMenu::~TextElideModeMenu() {
  const QString& textElideModeKey = GetName();
  if (!textElideModeKey.isEmpty()) {
    Configuration().setValue(textElideModeKey, GetTextElideMode());
  }
}

void TextElideModeMenu::onActionInMenuTriggered(const QAction* pTextElideModeAct) {
  CHECK_NULLPTR_RETURN_VOID(pTextElideModeAct);
  const Qt::TextElideMode newMode = mTextEditModeIntAction.act2Enum(pTextElideModeAct);
  emit reqTextElideModeChanged(newMode);
}

Qt::TextElideMode TextElideModeMenu::GetTextElideMode() const {
  return mTextEditModeIntAction.curVal();
}
