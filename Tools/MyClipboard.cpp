#include "MyClipboard.h"
#include "MimeDataCX.h"
#include <QGuiApplication>

MyClipboard::MyClipboard(QObject* parent)                           //
    : QObject{parent}, m_clipboard(QGuiApplication::clipboard()) {  //
}

int MyClipboard::FillClipboardFromSelectionInfo(const PathTool::SelectionInfo& info, const CCMMode::Mode cutCopy) {
  MimeDataCX* mimedata = new MimeDataCX{info, cutCopy};
  m_clipboard->setMimeData(mimedata);
  return info.relSelections.size();
}
