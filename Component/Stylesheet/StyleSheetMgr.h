#ifndef STYLESHEETMGR_H
#define STYLESHEETMGR_H
#include "StyleSheetTreeView.h"
#include "DialogWithSearchLine.h"
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSplitter>

class StyleSheetMgr : public DialogWithSearchLine {
public:
  explicit StyleSheetMgr(QWidget* parent = nullptr);
  ~StyleSheetMgr();
  void seeChanges();

private:
  void subscribe();
  void onStartFilter(const QString& searchText) override;
  void initStyleSheetPreview();
  QWidget* GetEffectPreviewer();
  void onApplyChanges();

  QAction *m_autoInitPreviewWindow{nullptr}, *m_initPreviewWindow{nullptr};
  QVBoxLayout* m_layout{nullptr};
  StyleSheetTreeView* m_styleSheetView{nullptr};
  QSplitter* m_spiltter{nullptr};

  QSplitter* m_effectSpiltter{nullptr};
  QWidget* m_effectLightPreviewer{nullptr};
  QWidget* m_effectDarkPreviewer{nullptr};

  QDialogButtonBox* m_dlgBtnBox{nullptr};
};

#endif // STYLESHEETMGR_H
