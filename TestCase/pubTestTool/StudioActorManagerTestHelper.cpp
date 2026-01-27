#include "StudioActorManagerTestHelper.h"
#include "PathTool.h"
#include "BeginToExposePrivateMember.h"
#include "StudiosManager.h"
#include "CastManager.h"
#include "EndToExposePrivateMember.h"
#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

#include "Logger.h"
StudioActorManagerTestHelper::StudioActorManagerTestHelper(TDir& tDir, const QString& fileName, const QString& blackFileName)
    : mTDir{tDir},
      mFileName{fileName},
      mBlackFileName{blackFileName},
      mFileAbsPath{tDir.itemPath(fileName)},
      mBlackFileAbsPath{tDir.itemPath(blackFileName)} {}

int StudioActorManagerTestHelper::setFileContents(const QByteArray& contents, const QByteArray& blackContents) {
  if (!mTDir.touch(mFileName, contents)) {
    LOG_W("write [%d]bytes into[%s] failed", contents.size(), qPrintable(mFileName));
    return -999;
  }
  if (!mTDir.touch(mBlackFileName, blackContents)) {
    LOG_W("write [%d]bytes into[%s] failed", blackContents.size(), qPrintable(mBlackFileName));
    return -998;
  }
  return reload();
}

ActorManagerTestHelper::ActorManagerTestHelper(TDir& tDir, const QString& fileName, const QString& blackFileName)
    : StudioActorManagerTestHelper{tDir, fileName, blackFileName} {}

void ActorManagerTestHelper::mock() {
  using namespace PathTool::FILE_REL_PATH;
  MOCKER(GetActorsListFilePath).stubs().will(returnValue(mFileAbsPath));
  MOCKER(GetMononymActorsListFilePath).stubs().will(returnValue(mBlackFileAbsPath));
}

int ActorManagerTestHelper::reload() {
  CastManager& actorMgr = CastManager::getInst();
  return actorMgr.ForceReloadImpl();
}

void ActorManagerTestHelper::init() {
  mock();
  CastManager& actorMgr = CastManager::getInst();
  using namespace PathTool::FILE_REL_PATH;
  actorMgr.InitializeImpl(GetActorsListFilePath(), GetMononymActorsListFilePath());
}

StudioManagerTestHelper::StudioManagerTestHelper(TDir& tDir, const QString& fileName, const QString& blackFileName)
    : StudioActorManagerTestHelper{tDir, fileName, blackFileName} {}

void StudioManagerTestHelper::mock() {
  using namespace PathTool::FILE_REL_PATH;
  MOCKER(GetVendorsTableFilePath).stubs().will(returnValue(mFileAbsPath));
  MOCKER(GetMononymVendorsListFilePath).stubs().will(returnValue(mBlackFileAbsPath));
}

int StudioManagerTestHelper::reload() {
  StudiosManager& studioMgr = StudiosManager::getInst();
  return studioMgr.ForceReloadImpl();
}

void StudioManagerTestHelper::init() {
  mock();
  StudiosManager& studioMgr = StudiosManager::getInst();
  using namespace PathTool::FILE_REL_PATH;
  studioMgr.InitializeImpl(GetVendorsTableFilePath(), GetMononymVendorsListFilePath());
}
