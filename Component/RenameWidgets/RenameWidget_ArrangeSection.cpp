#include "RenameWidget_ArrangeSection.h"
#include "Component/NotificatorFrame.h"
#include "PublicVariable.h"
#include "Tools/NameSectionArrange.h"
auto RenameWidget_ArrangeSection::extraSubscribe() -> void {
  connect(m_indexesCB, &QComboBox::currentTextChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
  connect(m_strictMode, &QCheckBox::stateChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
}

auto RenameWidget_ArrangeSection::InitExtraMemberWidget() -> void {
  const QString& defaultUserInput = PreferenceSettings().value(MemoryKey::RENAMER_ARRANGE_SECTION_INDEX.name, MemoryKey::RENAMER_ARRANGE_SECTION_INDEX.v).toString();
  m_indexesCB->addItem(defaultUserInput);
  m_indexesCB->addItems(NameSectionArrange::PATTERN_INDEX_FREQ);
  m_indexesCB->addItems(NameSectionArrange::SWAP_INDEX_FREQ);
}

auto RenameWidget_ArrangeSection::RenameCore(const QStringList& replaceeList) -> QStringList {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  bool isStrictMode = m_strictMode->isChecked();
  auto nsa = NameSectionArrange::FromUserInput(m_indexesCB->currentText(), isStrictMode);
  if (not nsa) {
    m_indexesCB->setStyleSheet(
        "QComboBox {"
        "    color: #fff;"
        "    background-color: rgb(200, 100, 20);"
        "    border-color: rgb(255, 0, 0);"
        "}");
    return replaceeList;
  }
  m_indexesCB->setStyleSheet("");
  PreferenceSettings().setValue(MemoryKey::RENAMER_ARRANGE_SECTION_INDEX.name, m_indexesCB->currentText());

  const QStringList& newNames = nsa.BatchSwapper(replaceeList);
  if (not nsa.m_wastedList.isEmpty()) {
    const QString& wastedNames = nsa.m_wastedList.join('|');
    Notificator::warning("wasted section found", wastedNames);
    qWarning("wasted section found[%s]", qPrintable(wastedNames));
  }
  return newNames;
}
