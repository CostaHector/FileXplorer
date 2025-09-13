#ifndef SEARCHMODETOOLBUTTON_H
#define SEARCHMODETOOLBUTTON_H

#include <QToolButton>
#include "EnumIntAction.h"
#include "SearchTools.h"

extern template struct EnumIntAction<SearchTools::SearchModeE>;

class SearchModeToolButton : public QToolButton{
  Q_OBJECT
public:
  explicit SearchModeToolButton(QWidget* parent=nullptr);
  ~SearchModeToolButton();
  SearchTools::SearchModeE curSearchMode() const {
    return mSearchModeIntAction.curVal();
  }

signals:
  void searchModeChanged(SearchTools::SearchModeE newSearchMode);

private:
  void EmitSearchModeChanged(QAction* checkedAction);

  QAction* MATCH_EQUAL{nullptr};
  QAction* MATCH_REGEX{nullptr};
  QAction* SEARCH_SCOPE_CONTENT{nullptr};
  EnumIntAction<SearchTools::SearchModeE> mSearchModeIntAction;
};

#endif // SEARCHMODETOOLBUTTON_H
