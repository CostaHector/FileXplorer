#include "TagEditorSideBar.h"
#include "TagsHelper.h"
#include "Logger.h"

TagEditorSideBar* TagEditorSideBar::GInstance(TagEditorSideBar* pInitValue, bool bReset) {
  static TagEditorSideBar* pInstance = pInitValue;
  if (bReset) {
    pInstance = pInitValue;
    return nullptr;
  }
  if (pInstance == nullptr) {
    if (pInitValue == nullptr) {
      LOG_W("Instance not init at all");
    } else {
      pInstance = pInitValue;
      LOG_W("Instance should init before here. adjust init position");
    }
  }
  return pInstance;
}

TagEditorSideBar::TagEditorSideBar(const QString &title, QWidget *parent)
  : QToolBar{title, parent} {
  m_tagEditor = new ActionsContainerWid{Qt::Orientation::Horizontal, 1, this};
  m_tagEditor->AddActions(TagsHelper::GetInst().GetActions(), Qt::ToolButtonTextBesideIcon);
  m_tagEditor->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

  setOrientation(Qt::Orientation::Vertical);
  addWidget(m_tagEditor);
}
