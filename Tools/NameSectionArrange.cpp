#include "NameSectionArrange.h"

const QStringList NameSectionArrange::PATTERN_INDEX_FREQ{"0213456", "0132456"};
const QStringList NameSectionArrange::SWAP_INDEX_FREQ{"1,2", "2,3"};
const int NameSectionArrange::INDEX_ARR[10]{0,1,2,3,4,5,6,7,8,9};

NameSectionArrange NameSectionArrange::FromUserInput(const QString& userInput, const bool strictMode) {
  const int commaCnt = userInput.count(',');
  if (commaCnt > 1){
    return NameSectionArrange();
  }
  if (commaCnt == 1){
    int ind = userInput.indexOf(',');
    bool is1stIndexInt;
    const int section1 = userInput.left(ind).toInt(&is1stIndexInt);
    bool is2ndIndexInt;
    const int section2 = userInput.mid(ind+1).toInt(&is2ndIndexInt);
    if (not is1stIndexInt or not is2ndIndexInt or section1 == section2 or section1 < 0 or section2 < 0 or section1 > 9 or section2 > 9) {
      qWarning("userInput[%s] is invalid[section1=%d, section2=%d]", qPrintable(userInput), section1, section2);
      return NameSectionArrange();
    }
    return NameSectionArrange{section1, section2, strictMode};
  }
  return NameSectionArrange{userInput, strictMode};
}
