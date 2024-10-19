#ifndef PROPERTIESWINDOW_H
#define PROPERTIESWINDOW_H

#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QToolBar>
#include <QSqlTableModel>
#include <QTableView>

class PropertiesWindow : public QDialog {
 public:
  explicit PropertiesWindow(QWidget* parent = nullptr);
  bool operator()(const QStringList& items);
  bool operator()(const QSqlTableModel* model, const QTableView* tv);

  void ReadSetting();
  void closeEvent(QCloseEvent* event) override;

 protected:
  bool UpdateMessage();

 private:
  void subscribe();

  void InitCommonInfo();
  void InitDurationInfo();
  void InitFileIndentifierInfo();

  QStringList m_items;
  QTextEdit* m_propertiesInfoTextEdit{new QTextEdit(this)};

  QVBoxLayout* m_mainLo{new QVBoxLayout{this}};
  QToolBar* m_propertyTB{nullptr};

  QString m_commonInfomation;
  QString m_durations;
  QString m_fileIdentifier;

  static const QString STRING_SPLITTER;
};

#endif  // PROPERTIESWINDOW_H

