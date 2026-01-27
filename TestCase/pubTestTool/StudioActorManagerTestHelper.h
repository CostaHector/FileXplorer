#ifndef STUDIOACTORMANAGERTESTHELPER_H
#define STUDIOACTORMANAGERTESTHELPER_H

#include "TDir.h"
class StudioActorManagerTestHelper {
 public:
  StudioActorManagerTestHelper(TDir& tDir, const QString& fileName, const QString& blackFileName);
  int setFileContents(const QByteArray& contents, const QByteArray& blackContents);
  virtual void init() {}
 protected:
  TDir& mTDir;
  const QString mFileName, mBlackFileName;
  const QString mFileAbsPath, mBlackFileAbsPath;
 private:
  virtual void mock() {}
  virtual int reload() {return 0;}
};


class ActorManagerTestHelper: public StudioActorManagerTestHelper {
 public:
  explicit ActorManagerTestHelper(TDir& tDir, const QString& fileName="Actors.txt", const QString& blackFileName="ActorsBlack.txt");
  void init() override;
 private:
  void mock() override;
  int reload() override;
};

class StudioManagerTestHelper: public StudioActorManagerTestHelper {
 public:
  explicit StudioManagerTestHelper(TDir& tDir, const QString& fileName="Studios.txt", const QString& blackFileName="StudiosBlack.txt");
  void init() override;
 private:
  void mock() override;
  int reload() override;
};



#endif  // STUDIOACTORMANAGERTESTHELPER_H
