#ifndef ADVANCERENAMER_H
#define ADVANCERENAMER_H

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QToolBar>
#include "StateLabel.h"
#include "MultiCursorEditor.h"

class FileOsWalker;
struct KV;

class AdvanceRenamer : public QDialog {
  Q_OBJECT
 public:
  static int execCore(AdvanceRenamer* self);
  explicit AdvanceRenamer(QWidget* parent = nullptr);

  void ReadSettings();

  void initCommonSetting();
  virtual void initExclusiveSetting() {}
  QHBoxLayout* GetNameEditsLayout();
  QDialogButtonBox* GetDlgButtonBox();
  void init();

  void Subscribe();
  bool GetApplyResult() const { return m_bApplyResult; }
  bool onApply(const bool isOnlyHelp = false);
  void onRegex(const int regexState);
  void onIncludingSub(int includingSubState);
  void onNameExtRespective(int bStateIndependent);

  virtual void FilterNames(FileOsWalker& /*walker*/) const {}
  void UpdateNameAndExt();
  void InitTextEditContent(const QString& workPath, const QStringList& selectedNames);
  void OnlyTriggerRenameCore();
  void setNewBaseNames(const QStringList& newBaseNames);
  QStringList GetSelectedFilesFullPath() const { return mSelectedFilesFullPath; }
  virtual void InitExtraCommonVariable() {}
  virtual void InitExtraMemberWidget() {}
  virtual QToolBar* InitControlTB() = 0;
  virtual void extraSubscribe() {}
  virtual QStringList RenameCore(const QStringList& replaceeList) = 0;

  QString windowTitleFormat = "%1 | %2";

 protected:
  static constexpr char NAME_SEP = '\n';
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

  QCheckBox* m_nameExtIndependent{nullptr};
  QCheckBox* m_recursiveCB{nullptr};
  StateLabel* regexValidLabel{nullptr};

  QString mWorkPath;
  QStringList mSelectedNames;
  QStringList mRelToNameWithNoRoot;  // (no root) relative path to file
  QStringList mSelectedFilesFullPath;

  QStringList mNames;  // with extension or without
  QStringList mExts;   // dot and extension

  QPlainTextEdit* m_relNameTE{nullptr};

  QPlainTextEdit* m_oBaseTE{nullptr};
  QPlainTextEdit* m_oExtTE{nullptr};
  // absolutefilepath = m_pre + m_relNameTE + m_oBaseTE + m_oExtTE
  MultiCursorEditor* m_nBaseTE{nullptr};
  QPlainTextEdit* m_nExtTE{nullptr};

  static QStringList MultiLineStr2StrList(const KV& kv);

 private:
  void SetApplyResult(bool applyResult) { m_bApplyResult = applyResult; }
  QTextBrowser* m_commandsPreview{nullptr};
  QDialogButtonBox* m_buttonBox{nullptr};

  QToolBar* m_controlBar{nullptr};
  QHBoxLayout* m_nameEditLayout{nullptr};
  QVBoxLayout* m_mainLayout{nullptr};
  bool m_bApplyResult{false};
};

#endif  // ADVANCERENAMER_H
