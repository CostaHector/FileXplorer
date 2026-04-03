#include "FontRegistry.h"
#include <QObject>

template <typename SetFontableObject>
typename FontRegistry<SetFontableObject>::TWidgetSet* FontRegistry<SetFontableObject>::GetWidgetsSet(bool useMonospaceFont) {
  static TWidgetSet regularFontWidgets;
  static TWidgetSet monospaceFontWidgets;
  return useMonospaceFont ? &monospaceFontWidgets : &regularFontWidgets;
}

template <typename SetFontableObject>
bool FontRegistry<SetFontableObject>::registerWidgetForFont(SetFontableObject* pWid, bool useMonospaceFont, bool isAlwaysAlive) {
  if (pWid == nullptr) {
    return false;
  }
  TWidgetSet* pSet{GetWidgetsSet(useMonospaceFont)};
  const auto& pr = pSet->insert(pWid);
  bool bRegisterResult = pr.second;
  if (bRegisterResult && !isAlwaysAlive) {
    QObject::connect(pWid, &QObject::destroyed, [pSet, pWid]() {
      auto it = pSet->find(pWid);
      if (it != pSet->end()) {
        pSet->erase(it);
      }
    });
  }
  return bRegisterResult;
}

template <typename SetFontableObject>
bool FontRegistry<SetFontableObject>::unregisterWidgetForFont(SetFontableObject* pWid, bool useMonospaceFont) {
  if (pWid == nullptr) {
    return false;
  }
  TWidgetSet* pSet{GetWidgetsSet(useMonospaceFont)};
  auto it = pSet->find(pWid);
  if (it == pSet->end()) {
    return false;
  }
  pSet->erase(it);
  return true;
}

template <typename SetFontableObject>
int FontRegistry<SetFontableObject>::updateRegisteredWidgetsFont(const QFont& newFont, bool useMonospaceFont) {
  int changedCnt = 0;
  TWidgetSet* pSet{GetWidgetsSet(useMonospaceFont)};
  for (auto* widget : *pSet) {
    if (widget == nullptr || widget->font() == newFont) {
      continue;
    }
    widget->setFont(newFont);
    ++changedCnt;
  }
  return changedCnt;
}

template <typename SetFontableObject>
FontRegistry<SetFontableObject>::AutoManager::AutoManager(SetFontableObject* _widget, bool _useMonospaceFont, bool _isAlwaysAlive)
    : widget{_widget}, useMonospaceFont{_useMonospaceFont}, isAlwaysAlive{_isAlwaysAlive} {
  registerWidgetForFont(_widget, _useMonospaceFont, true);
}

template <typename SetFontableObject>
FontRegistry<SetFontableObject>::AutoManager::~AutoManager() {
  if (!isAlwaysAlive) {
    unregisterWidgetForFont(widget, useMonospaceFont);
  }
}

#include <QWidget>
#include <QAction>
template struct FontRegistry<QWidget>;
template struct FontRegistry<QAction>;
