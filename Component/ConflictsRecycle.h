#ifndef CONFLICTSRECYCLE_H
#define CONFLICTSRECYCLE_H

#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QMenu>
#include <QPlainTextEdit>
#include <QToolBar>
#include <QVBoxLayout>

#include <QUrl>

#include "Model/ConflictsFileSystemModel.h"
#include "Tools/ConflictsItemHelper.h"
#include "View/CustomTableView.h"

class ConflictsRecycle : public QDialog {
  Q_OBJECT
 public:
  explicit ConflictsRecycle(const ConflictsItemHelper& itemIF_, QWidget* parent = nullptr);

  void ReadSettings();
  virtual void hideEvent(QHideEvent* event) override;
  virtual void closeEvent(QCloseEvent* event) override;

  const ConflictsItemHelper itemIF;  // dont user reference. object point to will be release after statement

  ConflictsFileSystemModel* m_conflictModel;
  CustomTableView* m_conflictTV;
  QDialogButtonBox* buttonBox{
      new QDialogButtonBox(QDialogButtonBox::StandardButton::Help | QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel)};

  QToolBar* m_conflictTB;
  QMenu* m_conflictMenu;

  QVBoxLayout* m_mainLayout{new QVBoxLayout{this}};
  QPlainTextEdit* m_commandsPreview{new QPlainTextEdit};

  auto on_completeMerge() -> bool;
  bool on_ShowCommand();

  bool on_doubleClicked();

  void Subscribe();
  bool on_Open(const QModelIndex& index);

  void keyPressEvent(QKeyEvent* e) override;

 private:
  bool onSetRecycleOrKeepChoiceByUser(const bool isSetDelete);
  bool onHideNoConflictLine(const bool isSetHide);
};
#endif  // CONFLICTSRECYCLE_H
