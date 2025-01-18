#include "RenameWidget_ReverseNames.h"
#include "Component/NotificatorFrame.h"
#include "PublicVariable.h"
#include "Tools/NameSectionArrange.h"
auto RenameWidget_ReverseNames::extraSubscribe() -> void {
}

auto RenameWidget_ReverseNames::InitExtraMemberWidget() -> void {
}

auto RenameWidget_ReverseNames::RenameCore(const QStringList& replaceeList) -> QStringList {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  QStringList replacedList{replaceeList.crbegin(), replaceeList.crend()};
  for (auto& name: replacedList) {
    name += ' ';
  }
  return replacedList;
}
