#ifndef PROPERTIESWINDOW_H
#define PROPERTIESWINDOW_H

#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QToolBar>
#include "FileSystemItemFilter.h"

class PropertiesWindow : public QDialog {
 public:
  explicit PropertiesWindow(QWidget* parent = nullptr);
  bool operator()(const QStringList& items);
  bool operator()(const QList<qint64>& fileSizes, const QList<int>& durations);

  void ReadSetting();
  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent* event) override;

 protected:
  bool UpdateMessage();

 private:
  void subscribe();

  void InitCommonInfo();
  bool InitDurationInfo();
  void InitFileIndentifierInfo();


  QTextEdit* m_propertiesInfoTextEdit{nullptr};
  QVBoxLayout* m_mainLo{nullptr};
  QToolBar* m_propertyTB{nullptr};

  QStringList mAllItems;
  FileSystemItemFilter::ItemStatistic mAllItemStatics;

  QString m_commonInfomation;
  QString m_durations;
  QString m_fileIdentifier;

  static const QString STRING_SPLITTER;
};

#endif  // PROPERTIESWINDOW_H

