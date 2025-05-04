#ifndef PERFORMERSWIDGET_H
#define PERFORMERSWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>

class JsonPerformersListInputer : public QDialog {
  Q_OBJECT
 public:
  explicit JsonPerformersListInputer(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  QString text() const { return m_perfsList->text(); }

  bool appendAPerformer();
  void uniquePerformers();
  bool submitPerformersListToJsonFile();
  bool reloadPerformersFromJsonFile(const QString& jsonFilePath, QVariantHash& dict);

  void subscribe();

  QSize sizeHint() const override { return QSize(600, 100); }

 private:
  QLineEdit* m_onePerf{nullptr};
  QLineEdit* m_perfsList{nullptr};
  QDialogButtonBox* buttonBox{nullptr};
  QVariantHash* p_dict{nullptr};

  QFormLayout* lo{nullptr};
};
#endif  // PERFORMERSWIDGET_H
