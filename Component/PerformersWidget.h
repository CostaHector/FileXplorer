#ifndef PERFORMERSWIDGET_H
#define PERFORMERSWIDGET_H

#include <QCompleter>
#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QDialogButtonBox>

class PerformerLineEditor : public QLineEdit {
  Q_OBJECT
 public:
  explicit PerformerLineEditor(QWidget* parent = nullptr);
};

class PerformersWidget : public QDialog {
  Q_OBJECT
 public:
  explicit PerformersWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  QString text() const { return m_perfsList->text(); }

  bool appendAPerformer();
  void uniquePerformers();
  bool submitPerformersListToJsonFile();
  bool reloadPerformersFromJsonFile(const QString& jsonFilePath, QVariantHash& dict);

  void subscribe();

  QSize sizeHint() const override{
    return QSize(600, 100);
  }

 private:
  PerformerLineEditor* m_onePerf;
  QLineEdit* m_perfsList;
  QDialogButtonBox *buttonBox;

  QVariantHash* p_dict;
};
#endif  // PERFORMERSWIDGET_H
