#ifndef SEARCHCASEMATTERTOOLBUTTON_H
#define SEARCHCASEMATTERTOOLBUTTON_H

#include <QToolButton>
#include <QAction>
#include <QMenu>

#include "Model/SearchProxyModel.h"

class SearchCaseMatterToolButton : public QToolButton {
 public:
  explicit SearchCaseMatterToolButton(QWidget* parent = nullptr);
  void BindSearchModel(SearchProxyModel* searchProxyModel);

 private:
  SearchProxyModel* _searchProxyModel = nullptr;
  QAction* SEARCH_NAME_CASE_SENSITIVE = new (std::nothrow) QAction("Name sensitive", this);
  QAction* SEARCH_CONTENTS_CASE_SENSITIVE = new (std::nothrow) QAction("Content sensitive", this);
  QMenu* m_searchCaseMenu = new QMenu{"Search case-sensitive", this};
};

#endif  // SEARCHCASEMATTERTOOLBUTTON_H
