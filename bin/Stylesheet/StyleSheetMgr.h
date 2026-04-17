#ifndef STYLESHEETMGR_H
#define STYLESHEETMGR_H
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include "StyleSheetTreeView.h"

class StyleSheetMgr : public QWidget {
public:
  explicit StyleSheetMgr(QWidget* parent = nullptr);

protected:
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;

private:
  void onStartFilter();

  QLineEdit* m_searchLineEdit{nullptr};
  StyleSheetTreeView* m_styleSheetView{nullptr};
  QVBoxLayout* m_layout{nullptr};

  QAction* m_startSearchAct{nullptr};
};

#endif // STYLESHEETMGR_H
