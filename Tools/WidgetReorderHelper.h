#ifndef WIDGETREORDERHELPER_H
#define WIDGETREORDERHELPER_H

#include <QVector>
enum class PREVIEW_ITEM_TYPE : int {  //
  BEGIN = 0,                          //
  IMG = BEGIN,                        //
  VID,                                //
  OTH,                                //
  BUTT                                //
};
bool MoveElementFrontOf(QVector<int>& v, int fromIndex, int destIndex);
bool IsValidMediaTypeSeq(const QString& seqStr, QVector<int>& result);
QString MediaTypeSeqStr(const QVector<int>& result);

#include <QSplitter>
#include <QBoxLayout>
#include <QToolBar>
bool MoveWidgetAtFromIndexInFrontOfDestIndex(int fromIndex, int destIndex, QToolBar& toolbar);
bool MoveWidgetAtFromIndexInFrontOfDestIndex(int fromIndex, int destIndex, QSplitter& splitter);
bool MoveWidgetAtFromIndexInFrontOfDestIndex(int fromIndex, int destIndex, QBoxLayout& layout);
#endif  // WIDGETREORDERHELPER_H
