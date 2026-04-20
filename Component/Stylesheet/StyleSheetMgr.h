#ifndef STYLESHEETMGR_H
#define STYLESHEETMGR_H
#include "StyleSheetTreeView.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSplitter>

class StyleSheetMgr : public QDialog {
public:
  explicit StyleSheetMgr(QWidget* parent = nullptr);
  void seeChanges();

protected:
  void showEvent(QShowEvent* event) override;

private:
  void subscribe();
  void onStartFilter();
  QWidget* GetEffectPreviewer();
  void onApplyChanges();

  QLineEdit* m_searchLineEdit{nullptr};
  QVBoxLayout* m_layout{nullptr};
  StyleSheetTreeView* m_styleSheetView{nullptr};
  QSplitter* m_spiltter{nullptr};

  QSplitter* m_effectSpiltter{nullptr};
  QWidget* m_effectLightPreviewer{nullptr};
  QWidget* m_effectDarkPreviewer{nullptr};

  QAction* m_startSearchAct{nullptr};
  QDialogButtonBox* m_dlgBtnBox{nullptr};
};

#endif // STYLESHEETMGR_H
