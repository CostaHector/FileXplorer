#ifndef SEARCHCASEMATTERTOOLBUTTON_H
#define SEARCHCASEMATTERTOOLBUTTON_H

#include <QToolButton>
#include <QAction>
#include <QMenu>

#include "Tools/SearchProxyModel.h"

class SearchCaseMatterToolButton:public QToolButton {
  Q_OBJECT
 public:
  explicit SearchCaseMatterToolButton(QWidget* parent=nullptr);
  void BindSearchModel(SearchProxyModel* searchProxyModel);
 private:
  SearchProxyModel* _searchProxyModel = nullptr;
  QAction* SEARCH_NAME_CASE_SENSITIVE = new (std::nothrow) QAction(tr("Name sensitive"), this);
  QAction* SEARCH_CONTENTS_CASE_SENSITIVE = new (std::nothrow) QAction(tr("Content sensitive"), this);
  QMenu* m_searchCaseMenu = new QMenu("Search case-sensitive");
};

#endif // SEARCHCASEMATTERTOOLBUTTON_H
