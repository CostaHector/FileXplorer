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

class FileOsWalker;

class AdvanceRenamer : public QDialog {
 public:
  explicit AdvanceRenamer(QWidget* parent = nullptr);

  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;
  void ReadSettings();

  void init();

  void Subscribe();
  bool onApply(const bool isOnlyHelp = false, const bool isInterative = false);
  void onRegex(const int regexState);
  void onIncludingSub(int includingSubState);
  void onNameExtRespective(int includingSuffixState);

  virtual void FilterNames(FileOsWalker& /*walker*/) const {}
  void UpdateNameAndExt();
  void InitTextEditContent(const QString& workPath, const QStringList& selectedNames);
  void OnlyTriggerRenameCore();

  virtual void InitExtraCommonVariable() = 0;
  virtual void InitExtraMemberWidget() = 0;
  virtual QToolBar* InitControlTB() = 0;
  virtual void extraSubscribe() = 0;
  virtual QStringList RenameCore(const QStringList& replaceeList) = 0;

 protected:
  static constexpr char NAME_SEP = '\n';

 public:
  QString windowTitleFormat;

  QString mWorkPath;
  QStringList mSelectedNames;
  QStringList mRelToNameWithNoRoot; // (no root) relative path to file
  QStringList mNames;  // with extension or without
  QStringList mExts;   // dot and extension

  QCheckBox* m_nameExtIndependent{nullptr};
  QCheckBox* m_recursiveCB{nullptr};
  StateLabel* regexValidLabel{nullptr};

  QPlainTextEdit* m_relNameTE{nullptr};

  QPlainTextEdit* m_oBaseTE{nullptr};
  QPlainTextEdit* m_oExtTE{nullptr};
  // absolutefilepath = m_pre + m_relNameTE + m_oBaseTE + m_oExtTE
  QPlainTextEdit* m_nBaseTE{nullptr};
  QPlainTextEdit* m_nExtTE{nullptr};

  QDialogButtonBox* m_buttonBox{nullptr};

  QToolBar* m_controlBar{nullptr};
  QHBoxLayout* m_nameEditLayout{nullptr};
  QVBoxLayout* m_mainLayout{nullptr};

  QPlainTextEdit* m_commandsPreview{nullptr};
};

#endif  // ADVANCERENAMER_H
