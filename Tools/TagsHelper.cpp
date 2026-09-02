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
  address->setProperty(TAG_ID_PROPERTY, textFullName);
  return address;
}

bool TagDefine::ParseALine(const QString& lineContent, TagDefine& tag) {
  QString trimmedLine = lineContent.trimmed();
  QStringList fields = trimmedLine.split(',', Qt::KeepEmptyParts);
  if (fields.size() == 1) {
    tag.textFullName = fields[0].trimmed();
    tag.textAbbr = tag.textFullName;
  } else if (fields.size() == 2) {
    tag.textFullName = fields[0].trimmed();
    tag.textAbbr     = fields[1].trimmed();
  } else if (fields.size() == 4) {
    tag.textFullName = fields[0].trimmed();
    tag.textAbbr     = fields[1].trimmed();
    tag.toolTip      = fields[2].trimmed();
    tag.accessCount  = fields[3].trimmed().toInt();
  } else {
    return false;
  }
  return true;
}

QList<TagDefine> TagDefine::ParseAFile(const QString& tagDefinitionCsvFilePath) {
  bool bReadResult{false};
  QString tagsCsvContent = FileTool::StringTextReader(tagDefinitionCsvFilePath, &bReadResult, true);
  if (!bReadResult) {
    tagsCsvContent = "textFullName,textAbbr,toolTip,accessCount\nDocumentary\nSuperhero\nComedy\n";
    LOG_W("File[%s] read failed", qPrintable(tagDefinitionCsvFilePath));
  }
  const QList<QString> tagLines = tagsCsvContent.split('\n', Qt::SkipEmptyParts);

  QList<TagDefine> tagsList;
  tagsList.reserve(tagLines.size());
  for (int i = 1; i < tagLines.size(); ++i) { // ignore the header line
    TagDefine tag;
    if (!ParseALine(tagLines[i], tag)) {
      LOG_W("The %dth Line[%s] parse failed", i + 1, qPrintable(tagLines[i]));
      continue;
    }
    tagsList.push_back(std::move(tag));
  }
  return tagsList;
}

QMap<QString, QString> GetTagActionText2IconPath() {
  const QString& resourceImagesPath{PathTool::Path2Join(SystemPath::CastStudioListPath(), "resources")};
  QDir dir{resourceImagesPath, "", QDir::SortFlag::NoSort, QDir::Filter::Files};
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  QMap<QString, QString> actionText2IconPath;
  for (const QString& imgName: dir.entryList()) {
    actionText2IconPath[PathTool::GetBaseName(imgName)] = PathTool::Path2Join(resourceImagesPath, imgName);
  }
  return actionText2IconPath;
}

TagsHelper& TagsHelper::GetInst() {
  static TagsHelper inst{SystemPath::GetMovieTagsListFilePath()};
  return inst;
}

TagsHelper::TagsHelper(const QString& tagDefinitionCsvFilePath, QObject* parent) : QObject{parent} {
  QMap<QString, QString> actionText2IconPath = GetTagActionText2IconPath();
  mTags = TagDefine::ParseAFile(tagDefinitionCsvFilePath);
  mTagsAG = new QActionGroup{this};
  mTagsAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
  InitActions();
  Subcribe();
}

void TagsHelper::UpdateTagsActionCheckedStatus(const QStringList& checkedTags) {
  for (TagDefine& tag: mTags) {
    bool bShouldChecked = checkedTags.contains(tag.textFullName, Qt::CaseInsensitive);
    if (tag.address == nullptr) {
      continue;
    }
    if (tag.address->isChecked() == bShouldChecked) {
      continue;
    }
    tag.address->setChecked(bShouldChecked);
  }
}

void TagsHelper::InitActions() {
  if (mTags.isEmpty()) {
    return;
  }
  const QMap<QString, QString> actionText2IconPath = GetTagActionText2IconPath();
  for (TagDefine& tag: mTags) {
    QAction* pAct = tag.InitAction(actionText2IconPath, this);
    mTagsAG->addAction(pAct);
    mTagActionList.push_back(pAct);
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
  emit reqAddRmvTags(tagId, pTagAction->isChecked());
  return true;
}