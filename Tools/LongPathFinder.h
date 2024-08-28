#ifndef LONGPATHFINDER_H
#define LONGPATHFINDER_H

#include <QString>
#include <QList>


class LongPathFinder
{
 public:
  LongPathFinder() = default;
  int operator()(const QString& path);
  static bool LengthBelow(const QString& path);

  void SetDropSectionWhenTooLong(int section);
  QString GetNewFolderName(const QString& oldName) const;
  int StillTooLongPathCount() const {
    return m_status.size() - pres.size();
  }
  int Check();

  QStringList pres;
  QStringList olds;
  QStringList news;
  QString m_status;
  int dropSection{-1};
  static int MAX_PATH_LENGTH;
};

#endif // LONGPATHFINDER_H
