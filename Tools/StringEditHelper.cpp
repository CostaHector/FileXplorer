#include "StringEditHelper.h"
#include <QTextDocumentFragment>

bool StringEditHelper::ReplaceAndUpdateSelection(QTextEdit* detailEditWidget, std::function<QString (QString)> fTrans) {
  if (detailEditWidget == nullptr) {
    return false;
  }
  if (not fTrans) {
    return false;
  }
  if (not detailEditWidget->textCursor().hasSelection()) {
    return true;
  }

  QTextCursor curSelection = detailEditWidget->textCursor();

  const int startPos = curSelection.selectionStart();

  const QString& before = curSelection.selection().toPlainText();
  curSelection.removeSelectedText();
  const QString& after = fTrans(before);
  curSelection.insertText(after);

  const int endPos = startPos + after.size();

  curSelection.setPosition(startPos);
  curSelection.setPosition(endPos, QTextCursor::KeepAnchor);
  detailEditWidget->setTextCursor(curSelection);
  return true;
}

bool StringEditHelper::ReplaceAndUpdateSelection(QLineEdit* lineEditWidget, std::function<QString (QString)> fTrans) {
  if (lineEditWidget == nullptr) {
    return false;
  }
  if (not fTrans) {
    return false;
  }
  if (not lineEditWidget->hasSelectedText()) {
    return true;
  }

  const int startPos = lineEditWidget->selectionStart();
  const int endPos = lineEditWidget->selectionEnd();

  const QString& before = lineEditWidget->selectedText();
  const QString& after = fTrans(before);
  const QString& beforeFullText = lineEditWidget->text();
  const QString& afterFullText = QString("%1%2%3").arg(beforeFullText.left(startPos), after, beforeFullText.mid(endPos));
  lineEditWidget->setText(afterFullText);

  const int newSelectLetterCnt = after.size();
  lineEditWidget->setSelection(startPos, newSelectLetterCnt);
  return true;
}
