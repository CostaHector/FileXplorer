#ifndef DUPLICATESIMAGESFINDER_H
#define DUPLICATESIMAGESFINDER_H

#include "DuplicateImagesTable.h"
#include <QDialog>
#include <QToolButton>
#include <QToolBar>
#include <QVBoxLayout>

class DuplicatesImagesFinder : public QDialog {
public:
  explicit DuplicatesImagesFinder(QWidget* parent = nullptr);
  bool operator()(const QString& folderPath);

protected:
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

private:
  void subscribe();
  void ReadSetting();

  QToolBar* m_toolBar{nullptr};
  DuplicateImagesTable* m_table{nullptr};
  QVBoxLayout* m_lo{nullptr};
};

#endif // DUPLICATESIMAGESFINDER_H
