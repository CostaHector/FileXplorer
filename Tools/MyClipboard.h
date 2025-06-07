#ifndef MYCLIPBOARD_H
#define MYCLIPBOARD_H

#include "public/PathTool.h"
#include "public/PublicVariable.h"
#include <QClipboard>

class MyClipboard : public QObject {
 public:
  explicit MyClipboard(QObject* parent = nullptr);
  int FillClipboardFromSelectionInfo(const PathTool::SelectionInfo& info, const CCMMode::Mode cutCopy);

  const QMimeData* mimeData(QClipboard::Mode /*mode*/ = QClipboard::Clipboard) const {  //
    return m_clipboard->mimeData();
  }

 private:
  QClipboard* m_clipboard;
};

#endif  // MYCLIPBOARD_H
