#ifndef RENAMEWIDGET_ARRANGESECTION_H
#define RENAMEWIDGET_ARRANGESECTION_H

#include "AdvanceRenamer.h"

class RenameWidget_ArrangeSection : public AdvanceRenamer {
 public:
  QComboBox* m_indexesCB{new QComboBox(this)};
  QCheckBox* m_strictMode{new QCheckBox("Strict mode", this)};
  explicit RenameWidget_ArrangeSection(QWidget* parent = nullptr) : AdvanceRenamer(parent) {
    EXT_INSIDE_FILENAME->setEnabled(false);
    EXT_INSIDE_FILENAME->setChecked(false);
    m_indexesCB->setCompleter(nullptr);
    m_indexesCB->setEditable(true);

    m_strictMode->setCheckable(true);
    m_strictMode->setChecked(true);
    m_strictMode->setToolTip("When enabled, if some section is wasted, name remain former");
  }
  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = "Arrange section sequence | %1 item(s) under [%2]";
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(":img/NAME_SECTIONS_SWAP"));
  }
  auto InitControlTB() -> QToolBar* override {
    QToolBar* replaceControl(new QToolBar);
    replaceControl->addWidget(new QLabel("Arrange index or swap 2 index:"));
    replaceControl->addWidget(m_indexesCB);
    replaceControl->addSeparator();
    replaceControl->addWidget(m_strictMode);
    replaceControl->addSeparator();
    replaceControl->addWidget(ITEMS_INSIDE_SUBDIR);
    replaceControl->addWidget(EXT_INSIDE_FILENAME);
    return replaceControl;
  }
  auto extraSubscribe() -> void override;
  auto InitExtraMemberWidget() -> void override;
  auto RenameCore(const QStringList& replaceeList) -> QStringList override;
};


#endif // RENAMEWIDGET_ARRANGESECTION_H
