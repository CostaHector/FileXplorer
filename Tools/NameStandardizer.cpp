#include "NameStandardizer.h"
#include "Tools/ProductionStudioManager.h"
#include "public/PublicVariable.h"

const QRegularExpression stdCommaComp("\\s+,");
const QRegularExpression stdExclamationComp("\\s+!");

using namespace JSON_RENAME_REGEX;
QString NameStandardizer::operator()(QString aFileName) {
  // non-standard character
  QString& noInvalidChar = aFileName.replace(invalidCharPat, " ");
  QString& noExtraSpaceComma = noInvalidChar.replace(stdCommaComp, ", ");
  QString& noExtraExclamationComma = noExtraSpaceComma.replace(stdExclamationComp, "! ");
  QString& noContiousSpace = noExtraExclamationComma.replace(continuousSpaceComp, " ");
  QString& noInvalidQuote = noContiousSpace.replace(invalidQuotePat, "'");

  QString& noLeadingStr = noInvalidQuote.remove(leadingStrComp);
  QString noLeadingBracket = noLeadingStr.trimmed().remove(leadingOpenBracketComp);

  QString& noBracket = noLeadingBracket.replace(nonLeadingBracketComp, "-");
  QString& standardStr = noBracket.replace(spaceBarSpaceComp, "-");
  QString& noContinousHypen = standardStr.replace(continousHypenComp, "-");

  QString& isloatedDot = noContinousHypen.replace(hypenOrSpaceFollowedWithDotPat, ".");
  QString& noHypenEnds = isloatedDot.remove(trailingHypenComp);
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
