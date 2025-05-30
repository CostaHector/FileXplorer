#include "RenameWidget_ArrangeSection.h"
#include "Component/Notificator.h"
#include "public/PublicMacro.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "Tools/NameSectionArrange.h"
#include <QLineEdit>

ArrangeSectionActions::ArrangeSectionActions(QObject* parent) : QObject{parent} {
  _SWAP_SECTION_AT_2_INDEXES->setCheckable(true);
  _SWAP_SECTION_AT_2_INDEXES->setChecked(true);
  _SECTION_JOIN_WITH_SELECT_INDEXES->setCheckable(true);
  _SECTION_JOIN_WITH_SELECT_INDEXES->setChecked(false);

  _ARRANGE_SECTION_AG->addAction(_SWAP_SECTION_AT_2_INDEXES);
  _ARRANGE_SECTION_AG->addAction(_SECTION_JOIN_WITH_SELECT_INDEXES);
  _ARRANGE_SECTION_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
}

ArrangeSectionActions& ArrangeSectionActions::GetInst() {
  static ArrangeSectionActions inst;
  return inst;
}

RenameWidget_ArrangeSection::RenameWidget_ArrangeSection(QWidget* parent)  //
    : AdvanceRenamer{parent} {
  m_2IndexesInput->setInputMask("0,0");
  m_2IndexesInput->setAlignment(Qt::AlignRight);
  m_indexes->setLineEdit(m_2IndexesInput);
  m_indexes->setEditable(true);
  m_indexes->setCompleter(nullptr);
  m_indexes->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  m_sectionsUsedToJoin->setCompleter(nullptr);
  m_sectionsUsedToJoin->setEditable(true);
  m_sectionsUsedToJoin->setToolTip("Section join with user input sequence");
  m_sectionsUsedToJoin->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  m_nameExtIndependent->setEnabled(false);
  m_nameExtIndependent->setChecked(false);

  m_strictMode->setCheckable(true);
  m_strictMode->setChecked(true);
  m_strictMode->setToolTip("If some section is wasted, name remains unchanged When enabled.");
}

auto RenameWidget_ArrangeSection::extraSubscribe() -> void {
  auto& inst = ArrangeSectionActions::GetInst();
  connect(inst._SWAP_SECTION_AT_2_INDEXES, &QAction::triggered,  //
          this, &AdvanceRenamer::OnlyTriggerRenameCore);
  connect(inst._SECTION_JOIN_WITH_SELECT_INDEXES, &QAction::triggered,  //
          this, &AdvanceRenamer::OnlyTriggerRenameCore);
  connect(m_indexes, &QComboBox::currentTextChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
  connect(m_sectionsUsedToJoin, &QComboBox::currentTextChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
  connect(m_strictMode, &QCheckBox::stateChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
}

auto RenameWidget_ArrangeSection::InitExtraMemberWidget() -> void {
  const QString& defaultUserInput = PreferenceSettings().value(MemoryKey::RENAMER_ARRANGE_SECTION_INDEX.name, MemoryKey::RENAMER_ARRANGE_SECTION_INDEX.v).toString();
  m_indexes->addItem(defaultUserInput);
  m_indexes->addItems(NameSectionArrange::SWAP_INDEX_FREQ);
}

QStringList RenameWidget_ArrangeSection::RenameCore(const QStringList& replaceeList) {
  NameSectionArrange nsa;

  auto& inst = ArrangeSectionActions::GetInst();
  const bool isStrictMode = m_strictMode->isChecked();
  QList<int> sortedSequenceIndex;
  if (inst._SWAP_SECTION_AT_2_INDEXES->isChecked()) {
    if (!SubscriptsStr2Int(m_indexes->currentText(), sortedSequenceIndex)) {
      qWarning("Swapped 2 indexes[%s] invalid", qPrintable(m_sectionsUsedToJoin->currentText()));
      regexValidLabel->ToNotSaved();
      return {};
    }
    if (sortedSequenceIndex.size() != 2) {
      qWarning("Swapped 2 indexes[%s] invalid must 2 number but now [%d]", qPrintable(m_sectionsUsedToJoin->currentText()), sortedSequenceIndex.size());
      regexValidLabel->ToNotSaved();
      return {};
    }
    PreferenceSettings().setValue(MemoryKey::RENAMER_ARRANGE_SECTION_INDEX.name, m_indexes->currentText());
    nsa = NameSectionArrange(sortedSequenceIndex.front(), sortedSequenceIndex.back(), isStrictMode);
  } else if (inst._SECTION_JOIN_WITH_SELECT_INDEXES->isChecked()) {
    if (!SubscriptsDigitChar2Int(m_sectionsUsedToJoin->currentText(), sortedSequenceIndex)) {
      qWarning("Sorted arrange indexes[%s] invalid", qPrintable(m_sectionsUsedToJoin->currentText()));
      regexValidLabel->ToNotSaved();
      return {};
    }
    nsa = NameSectionArrange(sortedSequenceIndex, isStrictMode);
  } else {
    qWarning("Section arrange style not found");
    regexValidLabel->ToNotSaved();
    return {};
  }

  regexValidLabel->ToSaved();

  const QStringList& newNames = nsa.BatchSwapper(replaceeList);
  if (isStrictMode && nsa.HasWasted()) {
    const QString& wastedNames = nsa.GetWastedNames();
    Notificator::warning("wasted section found", wastedNames);
    qWarning("Following name contains some section wasted. [%s]", qPrintable(wastedNames));
  }
  return newNames;
}

void RenameWidget_ArrangeSection::InitExtraCommonVariable() {
  windowTitleFormat = "Arrange section sequence | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_SECTIONS_SWAP"));
}

QToolBar* RenameWidget_ArrangeSection::InitControlTB() {
  QToolBar* arrangeControlTb{new (std::nothrow) QToolBar{"section arrange", this}};
  CHECK_NULLPTR_RETURN_NULLPTR(arrangeControlTb);
  auto& inst = ArrangeSectionActions::GetInst();
  arrangeControlTb->addAction(inst._SWAP_SECTION_AT_2_INDEXES);
  arrangeControlTb->addWidget(m_indexes);
  arrangeControlTb->addSeparator();
  arrangeControlTb->addAction(inst._SECTION_JOIN_WITH_SELECT_INDEXES);
  arrangeControlTb->addWidget(m_sectionsUsedToJoin);
  arrangeControlTb->addSeparator();
  arrangeControlTb->addWidget(m_strictMode);
  arrangeControlTb->addSeparator();
  arrangeControlTb->addWidget(m_recursiveCB);
  arrangeControlTb->addWidget(m_nameExtIndependent);
  arrangeControlTb->addWidget(regexValidLabel);
  return arrangeControlTb;
}
