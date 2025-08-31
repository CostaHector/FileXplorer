#include "RenameWidget_ArrangeSection.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include "NameSectionArrange.h"

RenameWidget_ArrangeSection::RenameWidget_ArrangeSection(QWidget* parent)  //
    : AdvanceRenamer{parent}                                               //
{ }

void RenameWidget_ArrangeSection::initExclusiveSetting() {
  m_nameExtIndependent->setEnabled(false);
  m_nameExtIndependent->setChecked(false);
}

auto RenameWidget_ArrangeSection::extraSubscribe() -> void {
  connect(_SWAP_SECTION_AT_2_INDEXES, &QAction::toggled, this, &AdvanceRenamer::OnlyTriggerRenameCore);
  connect(_SECTIONS_USED_TO_JOIN, &QAction::toggled, this, &AdvanceRenamer::OnlyTriggerRenameCore);
  connect(m_swap2Index, &QComboBox::currentTextChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
  connect(m_sectionsUsedToJoin, &QComboBox::currentTextChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
  connect(m_recordWasted, &QCheckBox::stateChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
}

auto RenameWidget_ArrangeSection::InitExtraMemberWidget() -> void {
  _SWAP_SECTION_AT_2_INDEXES = new (std::nothrow) QAction{"Swap 2 sections:", this};
  CHECK_NULLPTR_RETURN_VOID(_SWAP_SECTION_AT_2_INDEXES)
  _SWAP_SECTION_AT_2_INDEXES->setCheckable(true);
  _SWAP_SECTION_AT_2_INDEXES->setChecked(true);

  _SECTIONS_USED_TO_JOIN = new (std::nothrow) QAction{"Arrange sections:", this};
  CHECK_NULLPTR_RETURN_VOID(_SECTIONS_USED_TO_JOIN)
  _SECTIONS_USED_TO_JOIN->setCheckable(true);
  _SECTIONS_USED_TO_JOIN->setChecked(false);

  _ARRANGE_SECTION_AG = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(_ARRANGE_SECTION_AG)
  _ARRANGE_SECTION_AG->addAction(_SWAP_SECTION_AT_2_INDEXES);
  _ARRANGE_SECTION_AG->addAction(_SECTIONS_USED_TO_JOIN);
  _ARRANGE_SECTION_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  m_swap2Index = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_swap2Index)
  m_swap2Index->setEditable(true);
  m_swap2Index->setCompleter(nullptr);
  m_swap2Index->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
  m_swap2Index->setToolTip("Section at two indexes will be swapped");
  m_swap2Index->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  const QString& defaultUserInput = Configuration().value(MemoryKey::RENAMER_ARRANGE_SECTION_INDEX.name, MemoryKey::RENAMER_ARRANGE_SECTION_INDEX.v).toString();
  m_swap2Index->addItem(defaultUserInput);
  m_swap2Index->addItems(NameSectionArrange::SWAP_INDEX_FREQ);

  m_sectionsUsedToJoin = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_sectionsUsedToJoin)
  m_sectionsUsedToJoin->setEditable(true);
  m_sectionsUsedToJoin->setCompleter(nullptr);
  m_sectionsUsedToJoin->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
  m_sectionsUsedToJoin->setToolTip("Section will be joined with user input sequence");
  m_sectionsUsedToJoin->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  m_sectionsUsedToJoin->addItem("0123456789");

  m_recordWasted = new (std::nothrow) QCheckBox{"Record section wasted", this};
  CHECK_NULLPTR_RETURN_VOID(m_recordWasted)
  m_recordWasted->setCheckable(true);
  m_recordWasted->setChecked(true);
  m_recordWasted->setToolTip("If some section(s) is wasted, record it.");
}

QStringList RenameWidget_ArrangeSection::RenameCore(const QStringList& replaceeList) {
  NameSectionArrange nsa;

  const bool bRecordWasted = m_recordWasted->isChecked();
  QList<int> sortedSequenceIndex;
  if (_SWAP_SECTION_AT_2_INDEXES->isChecked()) {
    if (!SubscriptsStr2Int(m_swap2Index->currentText(), sortedSequenceIndex)) {
      qWarning("Swapped 2 indexes[%s] invalid, not number element find", qPrintable(m_sectionsUsedToJoin->currentText()));
      regexValidLabel->ToNotSaved();
      return {};
    }
    if (sortedSequenceIndex.size() != 2) {
      qWarning("Swapped 2 indexes[%s] invalid must 2 number but [%d] number(s) in factor", qPrintable(m_sectionsUsedToJoin->currentText()), sortedSequenceIndex.size());
      regexValidLabel->ToNotSaved();
      return {};
    }
#ifndef RUNNING_UNIT_TESTS
    Configuration().setValue(MemoryKey::RENAMER_ARRANGE_SECTION_INDEX.name, m_swap2Index->currentText());
#endif
    nsa = NameSectionArrange(sortedSequenceIndex.front(), sortedSequenceIndex.back(), bRecordWasted);
  } else if (_SECTIONS_USED_TO_JOIN->isChecked()) {
    if (!SubscriptsDigitChar2Int(m_sectionsUsedToJoin->currentText(), sortedSequenceIndex)) {
      qWarning("Sorted arrange indexes[%s] invalid", qPrintable(m_sectionsUsedToJoin->currentText()));
      regexValidLabel->ToNotSaved();
      return {};
    }
    nsa = NameSectionArrange(sortedSequenceIndex, bRecordWasted);
  } else {
    qWarning("Section arrange method not found");
    regexValidLabel->ToNotSaved();
    return {};
  }

  regexValidLabel->ToSaved();

  const QStringList& newNames = nsa.BatchSwapper(replaceeList);
  if (bRecordWasted && nsa.HasWasted()) {
    const QString& wastedNames = nsa.GetWastedNames();
    LOG_WARN_NP("wasted section found as follows:", wastedNames);
  }
  return newNames;
}

void RenameWidget_ArrangeSection::InitExtraCommonVariable() {
  windowTitleFormat = "Arrange section sequence | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_SECTIONS_ARRANGE"));
}

QToolBar* RenameWidget_ArrangeSection::InitControlTB() {
  QToolBar* arrangeControlTb{new (std::nothrow) QToolBar{"section arrange", this}};
  CHECK_NULLPTR_RETURN_NULLPTR(arrangeControlTb);
  arrangeControlTb->addAction(_SWAP_SECTION_AT_2_INDEXES);
  arrangeControlTb->addWidget(m_swap2Index);
  arrangeControlTb->addSeparator();
  arrangeControlTb->addAction(_SECTIONS_USED_TO_JOIN);
  arrangeControlTb->addWidget(m_sectionsUsedToJoin);
  arrangeControlTb->addSeparator();
  arrangeControlTb->addWidget(m_recordWasted);
  arrangeControlTb->addSeparator();
  arrangeControlTb->addWidget(m_recursiveCB);
  arrangeControlTb->addWidget(m_nameExtIndependent);
  arrangeControlTb->addWidget(regexValidLabel);
  return arrangeControlTb;
}
