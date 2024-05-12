#include "FileOperatorPub.h"

QString FileOperatorType::BatchCommands2String(const BATCH_COMMAND_LIST_TYPE& cmds) {
  QString s; // don't print this in debug
  for (const auto& strLst : cmds) {
    s += (strLst.join('\t'));
    s += '\n';
  }
  return s;
}
