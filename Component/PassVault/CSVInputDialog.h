#ifndef CSVINPUTDIALOG_H
#define CSVINPUTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include "AccountInfo.h"

class DragDropTextEdit : public QPlainTextEdit {
  Q_OBJECT
public:
  explicit DragDropTextEdit(const QString& aesKey, QWidget *parent = nullptr);

protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;

private:
  static bool isContainsOneFile(const QMimeData &mimeData);
  bool onSelectEncCsvFileToParse();
  bool ParseEncryptCsvFileContents(const QString& encryptCsvFilePath);

  QAction *mSelectEncCsvFile{nullptr};
  QMenu *mMenu{nullptr};
  const QString& mAesKey;
};

class CSVInputDialog : public QDialog {
  Q_OBJECT
public:
  explicit CSVInputDialog(const QString& aesKey, QWidget *parent = nullptr);
  QList<AccountInfo> GetAppendAccounts() const {
    return mAppendAccounts;
  }

  QSize sizeHint() const override { return QSize{1024, 720}; }

private:
  bool onParseRequest();
  void onAcceptAndQuit();

  const QString& mAesKey;

  QList<AccountInfo> mAppendAccounts;

  QPlainTextEdit *textEdit{nullptr};
  QDialogButtonBox *buttonBox{nullptr};
  QPushButton *pOkBtn{nullptr}, *pParseBtn{nullptr};
  QVBoxLayout *mainLayout{nullptr};
};

#endif // CSVINPUTDIALOG_H
