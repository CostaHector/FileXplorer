#ifndef CSVINPUTDIALOG_H
#define CSVINPUTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include "AccountInfo.h"

class DragDropTextEdit : public QTextEdit {
  Q_OBJECT
public:
  explicit DragDropTextEdit(QWidget *parent = nullptr);

protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;

private:
  static bool isContainsOneFile(const QMimeData &mimeData);
  bool onSelectEncCsvFileToParse();
  bool ParseEncryptCsvFileContents(const QString encryptCsvFilePath);

  QMenu *mMenu{nullptr};
};

class CSVInputDialog : public QDialog {
public:
  explicit CSVInputDialog(QWidget *parent = nullptr);
  QSize sizeHint() const override { return QSize{1024, 720}; }
  void raise();
  QVector<AccountInfo> tempAccounts;

private:
  void onHelpRequest();
  void onContentsChanged();
  void onCheckPassed();
  QTextEdit *textEdit{nullptr};
  QDialogButtonBox *buttonBox{nullptr};
  QPushButton *pOkBtn{nullptr}, *pCancelBtn{nullptr}, *pHelpBtn{nullptr};
  QVBoxLayout *mainLayout{nullptr};
};

#endif // CSVINPUTDIALOG_H
