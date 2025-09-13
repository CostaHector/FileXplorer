#include "SearchModeToolButton.h"
#include "MemoryKey.h"
#include <QMenu>

SearchModeToolButton::SearchModeToolButton(QWidget* parent) : QToolButton{parent} {
  using namespace SearchTools;
  MATCH_EQUAL          = new (std::nothrow) QAction{QIcon(":img/MATCH_EQUAL"), c_str(SEARCH_MODE::NORMAL)};
  MATCH_EQUAL->setCheckable(true);

  MATCH_REGEX          = new (std::nothrow) QAction{QIcon(":img/MATCH_REGEX"), c_str(SEARCH_MODE::REGEX)};
  MATCH_REGEX->setCheckable(true);

  SEARCH_SCOPE_CONTENT = new (std::nothrow) QAction{QIcon(":img/SEARCH_SCOPE_CONTENT"), c_str(SEARCH_MODE::FILE_CONTENTS)};
  SEARCH_SCOPE_CONTENT->setCheckable(true);

  mSearchModeIntAction.init({{MATCH_EQUAL, SEARCH_MODE::NORMAL},
                             {MATCH_REGEX, SEARCH_MODE::REGEX},
                             {SEARCH_SCOPE_CONTENT, SEARCH_MODE::FILE_CONTENTS}},//
                            SEARCH_MODE::NORMAL, QActionGroup::ExclusionPolicy::Exclusive);
  int searchModeInt = Configuration().value(MemoryKey::ADVANCE_SEARCH_MODE.name, (int)mSearchModeIntAction.defVal()).toInt();
  QAction* defaultModeAct = mSearchModeIntAction.setCheckedIfActionExist(searchModeInt);
  if (defaultModeAct != nullptr) {
    setDefaultAction(defaultModeAct); //
  } else {
    LOG_E("searchModeInt[%d] related to a nullptr", searchModeInt);
  }

  QMenu* pMenu = new (std::nothrow) QMenu{"Search Mode Menu", this};
  pMenu->addActions(mSearchModeIntAction.getActionEnumAscendingList());
  pMenu->setToolTipsVisible(true);

  setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  setAutoRaise(true);
  setMenu(pMenu);
  setToolTip("Search Mode:\n1. full match\n2. regex\n3. regex+file name");

  connect(this, &QToolButton::triggered, this, &SearchModeToolButton::EmitSearchModeChanged);
}

SearchModeToolButton::~SearchModeToolButton() {
  Configuration().setValue(MemoryKey::ADVANCE_SEARCH_MODE.name, (int)curSearchMode());
}

void SearchModeToolButton::EmitSearchModeChanged(QAction* checkedAction) {
  CHECK_NULLPTR_RETURN_VOID(checkedAction);

  SearchTools::SEARCH_MODE newSearchMode = mSearchModeIntAction.act2Enum(checkedAction);
  setDefaultAction(checkedAction); // default action follow current
  emit searchModeChanged(newSearchMode);
}
