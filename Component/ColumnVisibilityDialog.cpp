#include "ColumnVisibilityDialog.h"
#include "PublicMacro.h"
#include "StyleSheet.h"
#include <QLabel>
#include <QAction>
#include <QMenu>

ColumnVisibilityDialog::ColumnVisibilityDialog(const QStringList& headers,
                                               const QString& initSwitches,
                                               const QString& name,
                                               QWidget* parent) :
  QDialog{parent}
{
  if (initSwitches.size() < headers.size()) {
    qWarning("headers count[%d] out of switches count[%d] bound", headers.size(), headers.size());
    return;
  }

  QAction *mSelectAll{new (std::nothrow) QAction{QIcon(":img/SELECT_ALL"), "All", this}};
  QAction *mDeselectAll{new (std::nothrow) QAction{QIcon(":img/SELECT_NONE"), "None", this}};
  QAction *mInvertSelect{new (std::nothrow) QAction{QIcon(":img/SELECT_INVERT"), "Invert", this}};
  QAction *mRevertChange{new (std::nothrow) QAction{QIcon(":img/REVERT"), "Revert", this}};

  QMenu* mSelectMenu{new (std::nothrow) QMenu{"Select Menu", this}};
  CHECK_NULLPTR_RETURN_VOID(mSelectMenu)
  mSelectMenu->addAction(mSelectAll);
  mSelectMenu->addAction(mDeselectAll);
  mSelectMenu->addAction(mInvertSelect);
  mSelectMenu->addAction(mRevertChange);

  mSelectToolButton = new (std::nothrow) QToolButton{this};
  CHECK_NULLPTR_RETURN_VOID(mSelectToolButton)
  mSelectToolButton->setText("Quick Multi-Selection");
  mSelectToolButton->setIcon(QIcon(":img/QUICK_MULTI_SELECT"));
  mSelectToolButton->setMenu(mSelectMenu);
  mSelectToolButton->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
  mSelectToolButton->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);

  buttons = new (std::nothrow) QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  CHECK_NULLPTR_RETURN_VOID(buttons)

  m_layout = new (std::nothrow) QFormLayout{this};
  CHECK_NULLPTR_RETURN_VOID(m_layout)
  m_layout->addWidget(mSelectToolButton);


  m_layout->addRow("ID", new QLabel{"Column Name", this});
  m_checkboxes.reserve(headers.size());
  for (int i = 0; i < headers.size(); ++i) {
    QCheckBox* checkbox = new (std::nothrow) QCheckBox(headers[i], this);
    CHECK_NULLPTR_RETURN_VOID(checkbox)
    checkbox->setChecked(initSwitches[i] == '1');
    m_checkboxes.append(checkbox);
    m_layout->addRow(QString::number(i), checkbox);
  }
  m_layout->addWidget(buttons);

  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(mSelectAll, &QAction::triggered, this, [this](){ setAllCheckboxes(true); });
  connect(mDeselectAll, &QAction::triggered, this, [this](){ setAllCheckboxes(false); });
  connect(mInvertSelect, &QAction::triggered, this, &ColumnVisibilityDialog::toggleAllCheckboxes);
  connect(mRevertChange, &QAction::triggered, this, [this, initSwitches](){revertCheckboxes(initSwitches);});

  setMinimumWidth(400);
  setWindowIcon(QIcon(":img/COLUMN_VISIBILITY"));
  setWindowTitle(QString{"Column Visibility[%1]"}.arg(name));
}

void ColumnVisibilityDialog::setAllCheckboxes(bool checked)
{
  for (auto* checkbox : m_checkboxes) {
    if (checkbox != nullptr) checkbox->setChecked(checked);
  }
}

void ColumnVisibilityDialog::toggleAllCheckboxes()
{
  for (auto* checkbox : m_checkboxes) {
    if (checkbox != nullptr) checkbox->setChecked(!checkbox->isChecked());
  }
}

void ColumnVisibilityDialog::revertCheckboxes(const QString& initSwitches)
{
  for (int i = 0; i < m_checkboxes.size(); ++i) {
    if (m_checkboxes[i] != nullptr) {
      m_checkboxes[i]->setChecked(initSwitches[i] == '1');
    }
  }
}

QString ColumnVisibilityDialog::getSwitches() const {
  QString switches;
  switches.reserve(m_checkboxes.size());
  for (QCheckBox* checkbox : m_checkboxes) {
    switches.append(checkbox->isChecked() ? '1' : '0');
  }
  return switches;
}

void ColumnVisibilityDialog::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}
