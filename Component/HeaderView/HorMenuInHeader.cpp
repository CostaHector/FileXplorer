#include "HorMenuInHeader.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "ColumnVisibilityDialog.h"
#include "NotificatorMacro.h"

constexpr QChar HorMenuInHeader::SW_OFF;
constexpr QChar HorMenuInHeader::SW_ON;

const QString& HorMenuInHeader::DEFAULT_SWITCHES() {
  static const QString defSwitches(50, SW_ON);
  return defSwitches;
}

HorMenuInHeader::HorMenuInHeader(const QString &proName, QWidget *parent)
  : MenuInHeader{proName, Qt::Orientation::Horizontal, parent}
  , m_columnVisibiltyKey{GetName() + "_COLUMN_VISIBILITY"}
  , m_sortByColumnSwitchKey{GetName() + "_SORT_BY_COLUMN_SWITCH"}
  , m_columnsShowSwitch{Configuration().value(m_columnVisibiltyKey, DEFAULT_SWITCHES()).toString()} {
  _COLUMNS_VISIBILITY = new (std::nothrow) QAction(QIcon{":img/COLUMN_VISIBILITY"}, "Column visibility", this);
  _HIDE_THIS_COLUMN = new (std::nothrow) QAction(QIcon{":img/HIDE_THIS_COLUMN"}, "Hide this column", this);

  _ENABLE_COLUMN_SORT = new (std::nothrow) QAction(QIcon{":img/SORTING_FILE_FOLDER"}, "Enable column sort", this);
  _ENABLE_COLUMN_SORT->setCheckable(true);
  _ENABLE_COLUMN_SORT->setChecked(Configuration().value(m_sortByColumnSwitchKey, true).toBool());
  _ENABLE_COLUMN_SORT->setToolTip(QString("<b>%1 (%2)</b><br/> Enable/Disable sort by click on horizontal header")
                                     .arg(_ENABLE_COLUMN_SORT->text(), _ENABLE_COLUMN_SORT->shortcut().toString()));

  AddActionToMenu(nullptr);
  AddActionToMenu(_COLUMNS_VISIBILITY);
  AddActionToMenu(_HIDE_THIS_COLUMN);
  AddActionToMenu(nullptr);
  AddActionToMenu(_ENABLE_COLUMN_SORT);

  connect(_HIDE_THIS_COLUMN, &QAction::triggered, this, &HorMenuInHeader::onHideThisColumnTriggered);
  connect(_ENABLE_COLUMN_SORT, &QAction::toggled, this, &HorMenuInHeader::reqSortEnabled);
  connect(_COLUMNS_VISIBILITY, &QAction::triggered, this, &HorMenuInHeader::onColumnVisibilityAdjust);
}

HorMenuInHeader::~HorMenuInHeader() {
  Configuration().setValue(m_columnVisibiltyKey, m_columnsShowSwitch);
  Configuration().setValue(m_sortByColumnSwitchKey, _ENABLE_COLUMN_SORT->isChecked());
}

bool HorMenuInHeader::onHideThisColumnTriggered() {
  const int clickedSection = GetClickedSection();
  InvalidateClickedSection();
  if (!(0 <= clickedSection && clickedSection < m_columnsShowSwitch.size())) {
    LOG_WARN_P("[Skip] HideThisColumn", "Invalid column index[%d]", clickedSection);
    return false;
  }
  if (isColumnHidden(clickedSection)) {
    LOG_INFO_P("[Skip] Column already hide", "Select another column[not col %d] to hide", clickedSection);
    return true;
  }
  m_columnsShowSwitch[clickedSection] = SW_OFF;
  emit reqHideAColumn(clickedSection, true); // tableview call setColumnHidden(clickedSection, true);
  return true;
}

bool HorMenuInHeader::onColumnVisibilityAdjust() {
  auto *model_ = this->model();
  CHECK_NULLPTR_RETURN_FALSE(model_)

  const int tableColumnsCount{count()};
  QString tempSwitches;
  if (tableColumnsCount < m_columnsShowSwitch.size()) {
    tempSwitches = m_columnsShowSwitch.left(tableColumnsCount);
  } else {
    tempSwitches += m_columnsShowSwitch;
    tempSwitches += QString{tableColumnsCount - m_columnsShowSwitch.size(), SW_ON};
  }
  if (tempSwitches.size() != tableColumnsCount) {
    LOG_W("switches count[%d] should equal to column cnt[%d]", tempSwitches.size(), tableColumnsCount);
    return false;
  }

  // get column title(s)
#ifdef RUNNING_UNIT_TESTS
  m_columnsShowSwitch = UserSpecifiedIntValueMock::mockColumnsShowSwitch();
#else
  ColumnVisibilityDialog dialog{getTitles(), tempSwitches, GetName(), this};
  if (dialog.exec() != QDialog::Accepted) {
    LOG_I("User canceled column visibility change");
    return false;
  }
  m_columnsShowSwitch = dialog.getSwitches();
#endif

  emit reqUpdateColumnVisibilty();
  return true;
}

const QStringList &HorMenuInHeader::getTitles() const { //
  if (m_horHeaderTitles.isEmpty()) {
    if (const QAbstractItemModel *pModel = model()) {
      QStringList titlesTempList;
      const int columnCount = pModel->columnCount();
      for (int col = 0; col < columnCount; ++col) {
        const QString &headTitle = pModel->headerData(col, Qt::Orientation::Horizontal, Qt::DisplayRole).toString();
        titlesTempList.push_back(headTitle);
      }
      InitHorHeaderTitles(titlesTempList);
    }
  }
  return m_horHeaderTitles;
}

void HorMenuInHeader::InitHorHeaderTitles(QStringList &newTitles) const {
  m_horHeaderTitles.swap(newTitles);
}
