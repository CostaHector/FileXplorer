#include "TagsHelper.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "FileTool.h"
#include "SystemPath.h"
#include "PathTool.h"
#include "Logger.h"
#include <QDir>
#include <QAction>

constexpr const char* TAG_ID_PROPERTY = "TAG_ID";

QAction* TagDefine::InitAction(const QMap<QString, QString>& actionText2IconPath, QObject* parent) {
  if (address != nullptr) {
    return address;
  }
  QMap<QString, QString>::const_iterator it = actionText2IconPath.find(textFullName.toLower());
  if (it == actionText2IconPath.cend()) {
    address = new QAction{textAbbr, parent};
  } else {
    address = new QAction{QIcon{it.value()}, textAbbr, parent};
  }
  address->setCheckable(true);
  address->setToolTip(GetToolTip());
  address->setProperty(TAG_ID_PROPERTY, tagID());
  return address;
}

TagDefine TagDefine::ParseALine(const QString& lineContent) {
  QString trimmedLine = lineContent.trimmed();
  QStringList fields = trimmedLine.split(',', Qt::KeepEmptyParts);
  if (fields.size() == 1) {
    return TagDefine{fields[0].trimmed()};
  } else if (fields.size() == 2) {
    return TagDefine{fields[0].trimmed(), fields[1].trimmed()};
  } else if (fields.size() == 4) {
    return TagDefine{fields[0].trimmed(), fields[1].trimmed(), fields[2].trimmed(), fields[3].toInt()};
  } else {
    return {};
  }
}

QList<TagDefine> TagDefine::ParseAFile(const QString& tagDefinitionCsvFilePath) {
  bool bReadResult{false};
  QString tagsCsvContent = FileTool::StringTextReader(tagDefinitionCsvFilePath, &bReadResult, true);
  if (!bReadResult) {
    tagsCsvContent =
        R"(textFullName,textAbbr,toolTip,accessCount
Documentary
Superhero
Comedy
)";
    LOG_W("File[%s] read failed", qPrintable(tagDefinitionCsvFilePath));
  }
  const QList<QString> tagLines = tagsCsvContent.split('\n', Qt::SkipEmptyParts);

  QList<TagDefine> tagsList;
  tagsList.reserve(tagLines.size());
  for (int i = 1; i < tagLines.size(); ++i) { // ignore the header line
    TagDefine tag;
    if (!(tag = ParseALine(tagLines[i]))) {
      LOG_W("The %dth Line[%s] parse failed", i + 1, qPrintable(tagLines[i]));
      continue;
    }
    tagsList.push_back(std::move(tag));
  }
  return tagsList;
}

QMap<QString, QString> TagsHelper::GetImgBaseName2FullPath(const QString& resourceImagesPath) {
  QDir dir{resourceImagesPath, "", QDir::SortFlag::NoSort, QDir::Filter::Files};
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  QMap<QString, QString> actionText2IconPath;
  for (const QString& imgName: dir.entryList()) {
    actionText2IconPath[PathTool::GetBaseName(imgName).toLower()] = PathTool::Path2Join(resourceImagesPath, imgName);
  }
  return actionText2IconPath;
}

TagsHelper& TagsHelper::GetInst() {
  static TagsHelper inst{SystemPath::GetMovieTagsListFilePath()};
  return inst;
}

TagsHelper::TagsHelper(const QString& tagDefinitionCsvFilePath, QObject* parent) : QObject{parent} {
  mTags = TagDefine::ParseAFile(tagDefinitionCsvFilePath);
  mTagsAG = new QActionGroup{this};
  mTagsAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  InitActions();
  Subcribe();
}

// count of <cancelled check, new check>
std::pair<int, int> TagsHelper::UpdateTagsActionCheckedStatus(const QStringList& checkedTags) {
  int cancelledCheckCnt{0}, newCheckCnt{0};
  QSet<QString> checkedTagsId{checkedTags.cbegin(), checkedTags.cend()};
  // 1. update actions in mCurrentCheckedActions, update tags in input parms
  // let mCurrentCheckedActions={2,3}, checkedTagsId={1,2}
  auto itChecked{mCurrentCheckedActions.begin()};
  while (itChecked != mCurrentCheckedActions.end()) {
    QAction* pCheckedAct = *itChecked;

    auto itCheckedTags = checkedTagsId.find(pCheckedAct->text());
    if (itCheckedTags == checkedTagsId.end()) {
      // remove from mCurrentCheckedActions
      pCheckedAct->setChecked(false);
      itChecked = mCurrentCheckedActions.erase(itChecked);
      ++cancelledCheckCnt;
    } else {
      // remove from checkedTagsId
      checkedTagsId.erase(itCheckedTags);
      ++itChecked;
    }
  }

  // 2. update by tagID specified by checkedTags according mTagID2Action
  for (const QString& tagID: checkedTagsId) {
    auto it = mTagID2Action.find(tagID);
    if (it == mTagID2Action.end()) {
      continue;
    }
    QAction* needCheckedAct = it.value();
    needCheckedAct->setChecked(true);
    mCurrentCheckedActions.insert(needCheckedAct);
    ++newCheckCnt;
  }
  return {cancelledCheckCnt, newCheckCnt};
}

void TagsHelper::InitActions() {
  if (mTags.isEmpty()) {
    return;
  }
  const QMap<QString, QString> actionText2IconPath = GetImgBaseName2FullPath(PathTool::Path2Join(SystemPath::CastStudioListPath(), "resources"));
  for (TagDefine& tag: mTags) {
    QAction* pAct = tag.InitAction(actionText2IconPath, this);
    mTagsAG->addAction(pAct);
    mTagActionList.push_back(pAct);
    mTagID2Action[tag.tagID()] = pAct;
  }
}

void TagsHelper::Subcribe() {
  connect(mTagsAG, &QActionGroup::triggered, this, &TagsHelper::onActionGroupTriggered);
}

bool TagsHelper::onActionGroupTriggered(const QAction* pTagAction) {
  CHECK_NULLPTR_RETURN_FALSE(pTagAction);
  const QString tagId = pTagAction->property(TAG_ID_PROPERTY).toString();
  if (tagId.isEmpty()) {
    return false;
  }

  const bool bChecked{pTagAction->isChecked()};
  auto checkedOrUncheckedActIt = mCurrentCheckedActions.find(const_cast<QAction*>(pTagAction));
  if (checkedOrUncheckedActIt == mCurrentCheckedActions.end()) {
    if (bChecked) {
      mCurrentCheckedActions.insert(const_cast<QAction*>(pTagAction));
    } else {
      // no exist. already removed from set
    }
  } else {
    if (bChecked) {
      // exist. already checked
    } else {
      mCurrentCheckedActions.erase(checkedOrUncheckedActIt);
    }
  }
  emit reqAddRmvTags(tagId, pTagAction->isChecked());
  return true;
}