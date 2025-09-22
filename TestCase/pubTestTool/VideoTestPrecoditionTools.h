#ifndef VIDEOTESTPRECODITIONTOOLS_H
#define VIDEOTESTPRECODITIONTOOLS_H

#include "DupVidsManager.h"

struct VideoTestPrecoditionTools {
 public:
  static VideoTestPrecoditionTools& getInst() {
    static VideoTestPrecoditionTools inst;
    return inst;
  }
  const QString DUP_VID_DB = DupVidsManager::GetAiDupVidDbPath();
  const QString DUP_VID_CONN = DupVidsManager::VID_DUP_CONNECTION_NAME;
  const QString VID_DUR_GETTER_SAMPLE_PATH = TESTCASE_ROOT_PATH "/test/TestEnvVideosDurationGetter";
  const QString TS_FILE_MERGER_SAMPLE_PATH = TESTCASE_ROOT_PATH "/test/TestEnvTSFilesMerger";
 private:
  VideoTestPrecoditionTools() = default;
};

#endif  // VIDEOTESTPRECODITIONTOOLS_H
