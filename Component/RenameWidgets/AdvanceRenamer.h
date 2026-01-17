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
#include "StateLabel.h"
#include "MultiCursorEditor.h"

class FileOsWalker;

class AdvanceRenamer : public QDialog {
public:
  explicit AdvanceRenamer(QWidget* parent = nullptr);

  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;
  void ReadSettings();

  void initCommonSetting();
  virtual void initExclusiveSetting() {}
  QHBoxLayout* GetNameEditsLayout();
  QDialogButtonBox* GetDlgButtonBox();
  void init();

  void Subscribe();
  bool onApply(const bool isOnlyHelp = false);
  void onRegex(const int regexState);
  void onIncludingSub(int includingSubState);
  void onNameExtRespective(int bStateIndependent);

  virtual void FilterNames(FileOsWalker& /*walker*/) const {}
  void UpdateNameAndExt();
  void InitTextEditContent(const QString& workPath, const QStringList& selectedNames);
  void OnlyTriggerRenameCore();

  virtual void InitExtraCommonVariable() {}
  virtual void InitExtraMemberWidget() {}
  virtual QToolBar* InitControlTB() = 0;
  virtual void extraSubscribe() {}
  virtual QStringList RenameCore(const QStringList& replaceeList) = 0;

  QString windowTitleFormat = "%1 | %2";

protected:
  static constexpr char NAME_SEP = '\n';

  QCheckBox* m_nameExtIndependent{nullptr};
  QCheckBox* m_recursiveCB{nullptr};
  StateLabel* regexValidLabel{nullptr};

  QString mWorkPath;
  QStringList mSelectedNames;
  QStringList mRelToNameWithNoRoot; // (no root) relative path to file

  QStringList mNames;  // with extension or without
  QStringList mExts;   // dot and extension

  QPlainTextEdit* m_relNameTE{nullptr};

  QPlainTextEdit* m_oBaseTE{nullptr};
  QPlainTextEdit* m_oExtTE{nullptr};
  // absolutefilepath = m_pre + m_relNameTE + m_oBaseTE + m_oExtTE
  MultiCursorEditor* m_nBaseTE{nullptr};
  QPlainTextEdit* m_nExtTE{nullptr};

private:
  QPlainTextEdit* m_commandsPreview{nullptr};
  QDialogButtonBox* m_buttonBox{nullptr};

  QToolBar* m_controlBar{nullptr};
  QHBoxLayout* m_nameEditLayout{nullptr};
  QVBoxLayout* m_mainLayout{nullptr};
};

#endif  // ADVANCERENAMER_H
