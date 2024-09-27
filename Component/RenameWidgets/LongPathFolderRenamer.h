#ifndef LONGPATHFOLDERRENAMER_H
#define LONGPATHFOLDERRENAMER_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QPushButton>
#include <QLineEdit>
#include "Tools/LongPathFinder.h"

class LongPathFolderRenamer : public QDialog {
 public:
  explicit LongPathFolderRenamer(QWidget* parent = nullptr);

  virtual void closeEvent(QCloseEvent* event) override;
  void ReadSettings();

  void Subscribe();
  auto onApply(const bool isOnlyHelp = false, const bool isInterative = false) -> bool;
  auto InitTextContent(const QString& p) -> void;
  auto DropSectionChanged(const QString& newDropSectionStr) -> void;
  void UpdateStatus(const QStringList& news);

 public:
  QString windowTitleFormat;

  LongPathFinder m_lpf;

  QLineEdit* m_dropSectionLE{nullptr};
  QLineEdit* m_maxPathLengthLE{nullptr};
  QToolBar* m_dropControlBar{nullptr};

  QPlainTextEdit* m_statusTE{nullptr};
  QPlainTextEdit* m_preTE{nullptr};
  QPlainTextEdit* m_oldTE{nullptr};
  QPlainTextEdit* m_newTE{nullptr};
  QDialogButtonBox* m_buttonBox{nullptr};

  QHBoxLayout* m_nameEditLayout{nullptr};
  QVBoxLayout* m_mainLayout{nullptr};

  QPlainTextEdit* m_commandsPreview{nullptr};
};

#endif  // LONGPATHFOLDERRENAMER_H
