#ifndef TAGSHELPER_H
#define TAGSHELPER_H

#include <QString>
#include <QList>
#include <QObject>
#include <QSet>
class QAction;
class QActionGroup;

// csv file format (no double quote or comma charater in each column)
// ```csv
// textFullName,textAbbr,toolTip,accessCount.
// superhero,sprhr,eg. super man or bate men,0
// ```
struct TagDefine {
  TagDefine() = default;
  explicit TagDefine(const QString& _textFullName) : TagDefine{_textFullName, _textFullName} {
  }
  explicit TagDefine(const QString& _textFullName, const QString& _textAbbr, const QString& _toolTip="", int _accessCount=0)
    : textFullName{_textFullName}, textAbbr{_textAbbr}, toolTip{_toolTip}, accessCount{_accessCount}  {
  }
  bool isValid() const {return !textFullName.isEmpty(); }
  operator bool() const { return isValid(); }
  bool operator==(const TagDefine& rhs) const {
    return textFullName == rhs.textFullName && textAbbr == rhs.textAbbr && toolTip == rhs.toolTip && accessCount == rhs.accessCount && address == rhs.address;
  }

  // TagID aka textFullName
  QString textFullName, textAbbr;
  QString toolTip;
  int accessCount{0};
  QAction* address{nullptr};
  QAction* InitAction(const QMap<QString, QString>& actionText2IconPath, QObject* parent);
  QString GetToolTip() const {
    return QString{"<b>%1</b> %2<br/>Used count:%3"}.arg(textFullName, toolTip).arg(accessCount);
  }
  QString tagID() const { return textFullName; }
  static TagDefine ParseALine(const QString& lineContent);
  static QList<TagDefine> ParseAFile(const QString& tagDefinitionCsvFilePath);
};

class TagsHelper : public QObject {
  Q_OBJECT
public:
  static TagsHelper& GetInst();
  QList<QAction*> GetActions() const {
    return mTagActionList;
  }
  std::pair<int, int> UpdateTagsActionCheckedStatus(const QStringList& checkedTags);
  static QMap<QString, QString> GetImgBaseName2FullPath(const QString& resourceImagesPath);

signals:
  void reqAddRmvTags(const QString& tagID, bool bCheckOrUncheck);

private:
  explicit TagsHelper(const QString& tagDefinitionCsvFilePath, QObject* parent=nullptr);
  void InitActions();
  void Subcribe();
  bool onActionGroupTriggered(const QAction* pTagAction);

  QList<TagDefine> mTags;
  QList<QAction*> mTagActionList;
  QActionGroup* mTagsAG{nullptr};

  QSet<QAction*> mCurrentCheckedActions;
  QHash<QString, QAction*> mTagID2Action;
};

#endif // TAGSHELPER_H
