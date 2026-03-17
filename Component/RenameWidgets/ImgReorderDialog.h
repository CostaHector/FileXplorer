#ifndef IMGREORDERDIALOG_H
#define IMGREORDERDIALOG_H

#include <QDialog>
#include "ImgReorderListView.h"
#include <QDialogButtonBox>

class ImgReorderDialog : public QDialog {
  Q_OBJECT
 public:
  static int execCore(ImgReorderDialog* self);
  explicit ImgReorderDialog(QWidget* parent = nullptr);
  ~ImgReorderDialog();
  bool setImagesToReorder(const QStringList& files, const QString& baseName, int startNo = 0, const QString& pattern = " %1");
  QStringList getOrderedNames() const;

 protected:
  void showEvent(QShowEvent* event) override;

 private:
  void ReadSettings();

  ImgReorderListView* m_reorderListView{nullptr};
  QDialogButtonBox* m_buttonBox{nullptr};
};

#endif  // IMGREORDERDIALOG_H
