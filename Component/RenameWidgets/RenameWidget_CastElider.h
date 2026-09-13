#ifndef RENAMEWIDGET_CASTELIDER_H
#define RENAMEWIDGET_CASTELIDER_H

#include "AdvanceRenamer.h"
#include <QLineEdit>

class RenameWidget_CastElider : public AdvanceRenamer {
public:
  using AdvanceRenamer::AdvanceRenamer;
  ~RenameWidget_CastElider();

  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  void InitExtraMemberWidget() override;
  void initExclusiveSetting() override;

  QStringList RenameCore(const QStringList& replaceeList) override;
  void initCoreName2CastListMap(QMap<QString, QStringList>& initValue) {
    m_relativeJson2CastList.swap(initValue);
  }
  void initRelatedFile2Json(QMap<QString, QString>& initValue) {
    m_relatedFile2Json.swap(initValue);
  }

private:
  bool onJsonBaseNameMaxLengthChanged(const QString& maxNameLengthStr);

  static constexpr int JSON_BASENAME_LENGTH_MIN{0}, JSON_BASENAME_LENGTH_MAX{255};
  int m_JsonBaseNameMaxLength{0};
  QLineEdit* m_maxNameLength{nullptr}; // json file baseName length
  QMap<QString, QStringList> m_relativeJson2CastList;
  QMap<QString, QString> m_relatedFile2Json;
};

#endif // RENAMEWIDGET_CASTELIDER_H
