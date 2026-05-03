#ifndef HARMGR_H
#define HARMGR_H

#include <QWidget>
#include <QSplitter>
#include <QLineEdit>
#include <QVBoxLayout>
#include "HarTableView.h"
#include "ByteArrayImageViewer.h"

class HarMgr : public QWidget {
public:
  explicit HarMgr();
  int OpenHar(const QString& harAbsPath);

protected:
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

private:
  void subscribe();
  void onStartFilter();
  void ReadSetting();

  QLineEdit* m_searchLineEdit{nullptr};
  QAction* m_searchAction{nullptr};

  HarTableView* m_harView{nullptr};
  ByteArrayImageViewer* m_imgPreview{nullptr};

  QSplitter* m_splitterWid{nullptr};
  QVBoxLayout* m_lo{nullptr};
};

#endif // HARMGR_H
