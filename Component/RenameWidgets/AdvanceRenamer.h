#ifndef ADVANCERENAMER_H
#define ADVANCERENAMER_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSet>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include "Component/StateLabel.h"

class AdvanceRenamer : public QDialog {
 public:
  explicit AdvanceRenamer(QWidget* parent = nullptr);

  virtual void closeEvent(QCloseEvent* event) override;
  void ReadSettings();

  void init();

  void Subscribe();
  bool onApply(const bool isOnlyHelp = false, const bool isInterative = false);
  void onRegex(const int /*regexState*/);
  void onIncludingSub(int includingSubState);
  void onIncludeSuffix(int includingSuffixState);

  void InitTextContent(const QString& p, const QStringList& r);
  void OnlyTriggerRenameCore();

  virtual void InitExtraCommonVariable() = 0;
  virtual void InitExtraMemberWidget() = 0;
  virtual auto InitControlTB() -> QToolBar* = 0;
  virtual void extraSubscribe() = 0;
  virtual QStringList RenameCore(const QStringList& replaceeList) = 0;

 public:
  QString windowTitleFormat;

  QString m_pre;
  QStringList rels;
  QStringList completeNames;
  QList<bool> isFiles;

  QCheckBox* EXT_INSIDE_FILENAME;
  QCheckBox* ITEMS_INSIDE_SUBDIR;
  StateLabel* regexValidLabel;

  QPlainTextEdit* m_relNameTE;
  QPlainTextEdit* m_oBaseTE;
  QPlainTextEdit* m_oExtTE;  // absolutefilepath = m_pre + m_relNameTE + m_oBaseTE + m_oExtTE
  QPlainTextEdit* m_nBaseTE;
  QPlainTextEdit* m_nExtTE;
  QDialogButtonBox* m_buttonBox;

  QToolBar* m_replaceControlBar{nullptr};
  QHBoxLayout* m_nameEditLayout{nullptr};
  QVBoxLayout* m_mainLayout{nullptr};

  QPlainTextEdit* m_commandsPreview;
};

#endif  // ADVANCERENAMER_H
