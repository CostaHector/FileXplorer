#ifndef SEARCHMODETOOLBUTTON_H
#define SEARCHMODETOOLBUTTON_H

#include <QToolButton>
#include "EnumIntAction.h"
#include "SearchTools.h"

extern template struct EnumIntAction<SearchTools::SEARCH_MODE>;

class SearchModeToolButton : public QToolButton{
  Q_OBJECT
public:
  explicit SearchModeToolButton(QWidget* parent=nullptr);
  ~SearchModeToolButton();
  SearchTools::SEARCH_MODE curSearchMode() const {
    return mSearchModeIntAction.curVal();
  }

signals:
  void searchModeChanged(SearchTools::SEARCH_MODE newSearchMode);

private:
  void EmitSearchModeChanged(QAction* checkedAction);

  QAction* MATCH_EQUAL{nullptr};
  QAction* MATCH_REGEX{nullptr};
  QAction* SEARCH_SCOPE_CONTENT{nullptr};
  EnumIntAction<SearchTools::SEARCH_MODE> mSearchModeIntAction;
};

#endif // SEARCHMODETOOLBUTTON_H
