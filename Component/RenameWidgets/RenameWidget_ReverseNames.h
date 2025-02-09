#ifndef RENAMEWIDGET_REVERSENAMES_H
#define RENAMEWIDGET_REVERSENAMES_H

#include "AdvanceRenamer.h"

class RenameWidget_ReverseNames : public AdvanceRenamer {
 public:
  explicit RenameWidget_ReverseNames(QWidget* parent = nullptr) : AdvanceRenamer(parent) {
    EXT_INSIDE_FILENAME->setEnabled(false);
    EXT_INSIDE_FILENAME->setChecked(false);
  }
  auto InitExtraCommonVariable() -> void override {
    windowTitleFormat = "Reverse file names | %1 item(s) under [%2]";
    setWindowTitle(windowTitleFormat);
    setWindowIcon(QIcon(""));
  }
  auto InitControlTB() -> QToolBar* override {
    QToolBar* replaceControl(new QToolBar);
    replaceControl->addWidget(new QLabel("Reverse rename"));
    replaceControl->addSeparator();
    replaceControl->addWidget(ITEMS_INSIDE_SUBDIR);
    replaceControl->addWidget(EXT_INSIDE_FILENAME);
    return replaceControl;
  }
  auto extraSubscribe() -> void override;
  auto InitExtraMemberWidget() -> void override;
  auto RenameCore(const QStringList& replaceeList) -> QStringList override;
};

#endif // RENAMEWIDGET_REVERSENAMES_H
