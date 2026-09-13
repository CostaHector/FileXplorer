#include "RenameWidget_CastElider.h"
#include "PublicMacro.h"
#include "NameTool.h"
#include "Configuration.h"
#include "RenamerKey.h"

constexpr int RenameWidget_CastElider::JSON_BASENAME_LENGTH_MIN, RenameWidget_CastElider::JSON_BASENAME_LENGTH_MAX;

RenameWidget_CastElider::~RenameWidget_CastElider() {
  setConfig(RenamerKey::JSON_BASENAME_DEFAULT_MAX_LENGTH, m_JsonBaseNameMaxLength);
}

void RenameWidget_CastElider::initExclusiveSetting() {
  m_recursiveCB->setEnabled(false);
  m_recursiveCB->setChecked(false);

  m_nameExtIndependent->setEnabled(false);
  m_nameExtIndependent->setChecked(true);
}

QToolBar* RenameWidget_CastElider::InitControlTB() {
  auto* elideSectionTb = new (std::nothrow) QToolBar{"Drop Control Toolbar", this};
  CHECK_NULLPTR_RETURN_NULLPTR(elideSectionTb);
  auto* pMaxPathLength = new (std::nothrow) QLabel{tr("Json file basename MAX length:"), elideSectionTb};
  CHECK_NULLPTR_RETURN_NULLPTR(pMaxPathLength);

  elideSectionTb->addWidget(pMaxPathLength);
  elideSectionTb->addWidget(m_maxNameLength);
  elideSectionTb->addSeparator();
  elideSectionTb->addWidget(m_recursiveCB);
  elideSectionTb->addWidget(m_nameExtIndependent);
  return elideSectionTb;
}

void RenameWidget_CastElider::extraSubscribe() {
  connect(m_maxNameLength, &QLineEdit::textEdited, this, &RenameWidget_CastElider::onJsonBaseNameMaxLengthChanged);
}

void RenameWidget_CastElider::InitExtraMemberWidget() {
  m_JsonBaseNameMaxLength = getConfig(RenamerKey::JSON_BASENAME_DEFAULT_MAX_LENGTH).toInt();
  m_maxNameLength = new (std::nothrow) QLineEdit{QString::number(m_JsonBaseNameMaxLength), this};
  CHECK_NULLPTR_RETURN_VOID(m_maxNameLength);
}

void RenameWidget_CastElider::InitExtraCommonVariable() {
  windowTitleFormat = QString("Cast Elider | %1 item(s) under [%2]");
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/LONG_PATH_FINDER"));
}

bool RenameWidget_CastElider::onJsonBaseNameMaxLengthChanged(const QString& maxNameLengthStr) {
  bool isnumeric = false;
  const int maxNameLength = maxNameLengthStr.toInt(&isnumeric);
  if (!isnumeric) {
    LOG_W("Insert index[%s] must be a number, unchange", qPrintable(maxNameLengthStr));
    return false;
  }

  if (RenamerKey::JSON_BASENAME_DEFAULT_MAX_LENGTH.pChecker) {
    if (!RenamerKey::JSON_BASENAME_DEFAULT_MAX_LENGTH.pChecker(maxNameLength)) {
      LOG_W("Insert index[%d] out of range", maxNameLength);
      return false;
    }
  }
  if (m_JsonBaseNameMaxLength == maxNameLength) {
    return false;
  }
  m_JsonBaseNameMaxLength = maxNameLength;
  OnlyTriggerRenameCore();
  return true;
}

QStringList RenameWidget_CastElider::RenameCore(const QStringList& replaceeList) {
  decltype(m_relatedFile2Json)::const_iterator itFile2Json;
  decltype(m_relativeJson2CastList)::const_iterator itRelativeJson2Cast;

  QStringList newNames;
  newNames.reserve(replaceeList.size());
  for (int i = 0; i < replaceeList.size(); ++i) {
    // json相关文件的相对路径
    const QString& relativePath2relatedFile = relativePathAt(i);

    itFile2Json = m_relatedFile2Json.find(relativePath2relatedFile);
    if (itFile2Json == m_relatedFile2Json.cend()) {
      newNames.push_back("");
      LOG_W("Related json file not found using related file key[%s]", qPrintable(relativePath2relatedFile));
      continue;
    }
    // json自身文件相对路径
    const QString& relativeJsonFilePath = itFile2Json.value();

    itRelativeJson2Cast = m_relativeJson2CastList.find(relativeJsonFilePath);
    if (itRelativeJson2Cast == m_relativeJson2CastList.cend()) {
      newNames.push_back("");
      LOG_W("Cast list not found using relative json path key[%s]", qPrintable(relativeJsonFilePath));
      continue;
    }
    const QStringList& castList = itRelativeJson2Cast.value();

    // pre                           =       pre
    // pre/Marvel Films 2.jpg        =       relativePath2relatedFile
    // pre/Marvel Films.json         =       relativeJsonFilePath
    // e.g."
    // relativePath2relatedFile = "pre/Marvel Films 2.jpg":
    // newName:
    // [coreNameStartAt, coreNameEndAt)    +     castList            + [coreNameEndAt, end)
    // "Marvel Films"                      + " - cast 1, cast 2"     + " 2.jpg"
    // Marvel Films - cast 1, cast 2 2.jpg
    // Marvel Films - cast 1, cast.json
    const QString& pre = preAt(i);
    const int coreNameStartAt = pre.isEmpty() ? 0 : pre.size() + 1;
    const int coreNameEndAt = relativeJsonFilePath.size() - (sizeof(".json") - 1);
    const QString& coreName = relativePath2relatedFile.mid(coreNameStartAt, coreNameEndAt - coreNameStartAt);

    const int postfixWithoutExtLen = relativePath2relatedFile.size() - coreNameEndAt - mExts[i].size();
    newNames.push_back(NameTool::ComposeNameWithinLimit(coreName, castList, m_JsonBaseNameMaxLength)
                       + relativePath2relatedFile.mid(coreNameEndAt, postfixWithoutExtLen));
  }
  return newNames;
}