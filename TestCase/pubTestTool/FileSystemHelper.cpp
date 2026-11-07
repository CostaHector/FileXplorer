#include "FileSystemHelper.h"

bool FileSystemNode::operator==(const FileSystemNode& rhs) const {
  if (fileName != rhs.fileName) {
    return false;
  }
  if (isDir != rhs.isDir) {
    return false;
  }
  if (isDir) {
    return true;
  }
  return fileContents == rhs.fileContents;
}
