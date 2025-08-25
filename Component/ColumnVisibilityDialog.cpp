#include "ColumnVisibilityDialog.h"
#include "PublicMacro.h"
#include <QAction>
#include <QMenu>

ColumnVisibilityDialog::ColumnVisibilityDialog(const QStringList& headers,
                                               const QString& currentSwitches,
                                               QWidget* parent) :
  QDialog{parent}
{
  if (headers.size() > currentSwitches.size()) {
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

  m_layout = new (std::nothrow) QVBoxLayout{this};
  m_layout->addWidget(mSelectToolButton);
  CHECK_NULLPTR_RETURN_VOID(m_layout)
  for (int i = 0; i < headers.size(); ++i) {
    QCheckBox* checkbox = new (std::nothrow) QCheckBox(headers[i], this);
    CHECK_NULLPTR_RETURN_VOID(checkbox)
    checkbox->setChecked(currentSwitches[i] == '1');
    m_checkboxes.append(checkbox);
    m_layout->addWidget(checkbox);
  }
  m_layout->addWidget(buttons);

  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(mSelectAll, &QAction::triggered, this, [this](){
    for (auto* checkedBox: m_checkboxes) {
      if (checkedBox != nullptr) { checkedBox->setChecked(true); }
    }
  });
  connect(mDeselectAll, &QAction::triggered, this, [this](){
    for (auto* checkedBox: m_checkboxes) {
      if (checkedBox != nullptr) { checkedBox->setChecked(false); }
    }
  });
  connect(mInvertSelect, &QAction::triggered, this, [this](){
    for (auto* checkedBox: m_checkboxes) {
      if (checkedBox != nullptr) { checkedBox->setChecked(!checkedBox->isChecked()); }
    }
  });
  connect(mRevertChange, &QAction::triggered, this, [this, currentSwitches](){
    for (int i = 0; i < m_checkboxes.size(); ++i) {
      auto* checkedBox = m_checkboxes[i];
      if (checkedBox != nullptr) {
        checkedBox->setChecked(currentSwitches[i] == '1');
      }
    }
  });

  setWindowIcon(QIcon(":img/COLUMN_VISIBILITY"));
  setWindowTitle("Column Visibility Settings");
}


QString ColumnVisibilityDialog::getSwitches() const {
  QString switches;
  switches.reserve(m_checkboxes.size());
  for (QCheckBox* checkbox : m_checkboxes) {
    switches.append(checkbox->isChecked() ? '1' : '0');
  }
  return switches;
}
