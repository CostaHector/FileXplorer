#ifndef PERFORMERSWIDGET_H
#define PERFORMERSWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QCompleter>
#include <QVariantHash>

class JsonPerformersListInputer : public QDialog {
 public:
  explicit JsonPerformersListInputer(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  QSize sizeHint() const override;
  QString text() const { return m_perfsList->text(); }

  bool appendAPerformer();
  void uniquePerformers();
  bool submitPerformersListToJsonFile();
  bool reloadPerformersFromJsonFile(const QString& jsonFilePath, QVariantHash& dict);

  void subscribe();


 private:
  QLineEdit* m_onePerf{nullptr};
  QLineEdit* m_perfsList{nullptr};
  QDialogButtonBox* buttonBox{nullptr};
  QVariantHash* p_dict{nullptr};

  QFormLayout* lo{nullptr};
  QCompleter mPerfsCompleter;
};
#endif  // PERFORMERSWIDGET_H
