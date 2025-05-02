#include "NameStandardizer.h"
#include "Tools/ProductionStudioManager.h"
#include "public/PublicVariable.h"

const QRegularExpression stdCommaComp("\\s+,");
const QRegularExpression stdExclamationComp("\\s+!");

using namespace JSON_RENAME_REGEX;
QString NameStandardizer::operator()(QString aFileName) {
  // non-standard character
  QString& noInvalidChar = aFileName.replace(INVALID_CHARS_IN_FILENAME, " ");
  QString& noExtraSpaceComma = noInvalidChar.replace(stdCommaComp, ", ");
  QString& noExtraExclamationComma = noExtraSpaceComma.replace(stdExclamationComp, "! ");
  QString& noContiousSpace = noExtraExclamationComma.replace(CONTINOUS_SPACE_COMP, " ");
  QString& noInvalidQuote = noContiousSpace.replace(INVALID_QUOTE_IN_FILENAME, "'");

  QString& noLeadingStr = noInvalidQuote.remove(TORRENT_LEADING_STR_COMP);
  QString noLeadingBracket = noLeadingStr.trimmed().remove(LEADING_OPEN_BRACKET_COMP);

  QString& noBracket = noLeadingBracket.replace(NON_LEADING_BRACKET_COMP, "-");
  QString& standardStr = noBracket.replace(SPACE_HYPEN_SPACE_COMP, "-");
  QString& noContinousHypen = standardStr.replace(CONTINOUS_HYPEN_COMP, "-");

  QString& isloatedDot = noContinousHypen.replace(HYPEN_OR_SPACE_END_WITH_DOT_COMP, ".");
  QString& noHypenEnds = isloatedDot.remove(TRAILING_HYPEN_COMP);
  QString fileName = noHypenEnds.replace('-', " - ").trimmed();

  // Get standard Name
  const int barIndex = fileName.indexOf('-');
  if (barIndex == -1 || barIndex == 0) {
    return fileName;
  }
  static auto& psm = ProductionStudioManager::getIns();
  const QString& studioName = fileName.left(barIndex - 1);
  return psm[studioName] + fileName.mid(barIndex - 1);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
  const auto& mp = NameStandardizer::jsonLoader();
  qDebug("STANDARD_STUDIO_NAME size=%d", mp.size());
}
#endif
