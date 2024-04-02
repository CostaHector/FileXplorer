#include "ConvertBoldUnicodeCharsetToAscii.h"

const QHash<QString, char> ConvertBoldUnicodeCharsetToAscii::BOLD_TO_TEXT_DICT = getBoldToTextDict();

QHash<QString, char> ConvertBoldUnicodeCharsetToAscii::getBoldToTextDict() {
  const char* TEXT = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzCDEFGIKLMNOPSUREadeghilnorstw";
  const QString BOLD = "𝗔,𝗕,𝗖,𝗗,𝗘,𝗙,𝗚,𝗛,𝗜,𝗝,𝗞,𝗟,𝗠,𝗡,𝗢,𝗣,𝗤,𝗥,𝗦,𝗧,𝗨,𝗩,𝗪,𝗫,𝗬,𝗭,𝗮,𝗯,𝗰,𝗱,𝗲,𝗳,𝗴,𝗵,𝗶,𝗷,𝗸,𝗹,𝗺,𝗻,𝗼,𝗽,𝗾,𝗿,𝘀,𝘁,𝘂,𝘃,𝘄,𝘅,𝘆,𝘇,𝐂,𝗗,𝗘,𝐅,𝐆,𝐈,𝐊,𝐋,𝗠,𝐍,𝐎,𝐏,𝐒,𝐔,𝐑,𝐄,𝐚,𝐝,𝐞,𝐠,𝐡,𝐢,𝐥,𝐧,𝐨,𝐫,𝐬,𝐭,𝐰";
  QStringList BOLDLST = BOLD.split(',');
  if (strlen(TEXT) != BOLDLST.size()){
    qCritical("TEXT[%d] and BOLDLST[%d] not same length", strlen(TEXT), BOLDLST.size());
    return {};
  }
  QHash<QString, char> dict;
  dict.reserve(BOLD.size());
  for (int i = 0; i < BOLDLST.size(); ++i){
    dict.insert(BOLDLST[i], TEXT[i]);
  }
  return dict;
}
