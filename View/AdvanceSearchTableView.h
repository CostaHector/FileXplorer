#ifndef ADVANCESEARCHTABLEVIEW_H
#define ADVANCESEARCHTABLEVIEW_H

#include "AdvanceSearchModel.h"
#include "CustomTableView.h"
#include "Tools/SearchProxyModel.h"
#include "Component/CustomStatusBar.h"
#include <QDebug>
#include <QLineEdit>
#include <QMainWindow>
#include <QToolBar>

#include <QMenu>

class AdvanceSearchTableView : public CustomTableView {
 public:
  AdvanceSearchTableView(AdvanceSearchModel* sourceModel, SearchProxyModel* searchProxyModel, QWidget* parent = nullptr);

  void subscribe();

  void onCopyNamesStr() const;
  void onCopyFiles();
  void onCutFiles();

  void BindLogger(CustomStatusBar* logger);

 private:
  QAction* COPY_NAME_STR = new QAction("copy name str", this);
  QAction* COPY_ABSOLUTE_PATH_STR = new QAction("copy absolute path str", this);
  QAction* COPY_DIR_STR = new QAction("copy directory str", this);
  QAction* COPY_FILE = new QAction("copy file", this);
  QAction* CUT_FILE = new QAction("cut file", this);
  QMenu* m_menu = new QMenu("search menu", this);

  AdvanceSearchModel* _sourceModel;
  SearchProxyModel* _searchProxyModel;
  CustomStatusBar* _logger{nullptr};
};

#endif  // ADVANCESEARCHTABLEVIEW_H
