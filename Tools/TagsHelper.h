#ifndef TAGSHELPER_H
#define TAGSHELPER_H

#include <QString>
#include <QList>
#include <QObject>
class QAction;
class QActionGroup;

// csv file format (no double quote or comma charater in each column)
// ```csv
// textFullName,textAbbr,toolTip,accessCount.
// superhero,sprhr,eg. super man or bate men,0
// ```
struct TagDefine {
  // TagID aka textFullName
  QString textFullName, textAbbr;
  QString toolTip;
  int accessCount{0};
  QAction* address{nullptr};
  QAction* InitAction(const QMap<QString, QString>& actionText2IconPath, QObject* parent);
  QString GetToolTip() const {
    return QString{"<b>%1</b> %2<br/>Used count:%3"}.arg(textFullName, toolTip).arg(accessCount);
  }
  static bool ParseALine(const QString& lineContent, TagDefine& tag);
  static QList<TagDefine> ParseAFile(const QString& tagDefinitionCsvFilePath);
};

class TagsHelper : public QObject {
  Q_OBJECT
public:
  static TagsHelper& GetInst();
  QList<QAction*> GetActions() const {
    return mTagActionList;
  }
  void UpdateTagsActionCheckedStatus(const QStringList& checkedTags);

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
};

#endif // TAGSHELPER_H
