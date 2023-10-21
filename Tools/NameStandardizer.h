#ifndef NAMESTANDARDIZER_H
#define NAMESTANDARDIZER_H

#include <QString>
#include <QMap>



#include <QRegExp>
const QRegExp invalidCharPat("[\\#\\\\/\\:\\*\\?\\<\\>\\|]");  //#\/:*?<>
const QRegExp invalidQuotePat("[’“”\"]");  //'

const QRegExp leadingStrComp("^((\\[FL\\])|(\\[FFL\\])|(\\[GT\\]))");  //delete it
const QRegExp leadingOpenBracketComp("^[\[\{\(]");  //-

const QRegExp continuousSpaceComp("\\s\\s+");  //' '

const QRegExp leadingBracketComp("[\(\{\\[\\)\\}\\]–]");  //-
const QRegExp spaceBarSpaceComp("\\s*-\\s*");  //-
const QRegExp continousHypenComp("--+");  //-

const QRegExp hypenOrSpaceFollowedWithDotPat("[\\s-]\\.");  //.
const QRegExp trailingHypenComp("-$");  //delete it




class NameStandardizer
{
 public:
  static auto jsonLoader() -> QMap<QString, QString>;

  auto operator()(QString aFileName)->QString{
    static const QMap<QString, QString>& NAME_TABLE = jsonLoader();
    auto noInvalidChar = aFileName.replace(invalidCharPat, " ");
    auto noContiousSpace = noInvalidChar.replace(continuousSpaceComp, " ");
    auto noInvalidQuote = noContiousSpace.replace(invalidQuotePat, "'");

    //non-standard character

    auto noLeadingStr = noInvalidQuote.remove(leadingStrComp);

    auto noLeadingBracket = noLeadingStr.trimmed().remove(leadingOpenBracketComp);

    auto noBracket = noLeadingBracket.replace(leadingBracketComp, "-");
    auto standardStr = noBracket.replace(spaceBarSpaceComp, "-");
    auto noContinousHypen = standardStr.replace(continousHypenComp, "-");

    auto isloatedDot = noContinousHypen.replace(hypenOrSpaceFollowedWithDotPat, ".");
    auto noHypenEnds = isloatedDot.remove(trailingHypenComp);
    auto fileName = noHypenEnds.replace('-', " - ").trimmed();

    //Get standard Name
    auto barIndex = fileName.indexOf('-');
    if (barIndex == -1){
        return fileName;
    }
    auto studioName = fileName;
    studioName = fileName.left(barIndex - 1);
    const QString& lowerFileName = studioName.toLower();
    if (NAME_TABLE.contains(lowerFileName)){
        studioName = NAME_TABLE[lowerFileName];
    }
    return studioName + fileName.mid(barIndex - 1);
  }
};

#endif // NAMESTANDARDIZER_H
