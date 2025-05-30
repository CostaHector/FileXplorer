#include "RenameWidget_LongPath.h"
#include "public/PublicMacro.h"
#include "public/PathTool.h"
#include <QLabel>

RenameWidget_LongPath::RenameWidget_LongPath(QWidget* parent)  //
    : AdvanceRenamer{parent}                                   //
{
  m_dropSectionLE = new (std::nothrow) QLineEdit("-1", this);
  CHECK_NULLPTR_RETURN_VOID(m_dropSectionLE);
  m_maxPathLengthLE = new (std::nothrow) QLineEdit("260", this);
  CHECK_NULLPTR_RETURN_VOID(m_maxPathLengthLE);
}

QToolBar* RenameWidget_LongPath::InitControlTB() {
  auto* chopSectionTb = new (std::nothrow) QToolBar{"Drop Control Toolbar", this};
  CHECK_NULLPTR_RETURN_NULLPTR(chopSectionTb);
  auto* pDropSectionLabel = new (std::nothrow) QLabel{"Drop Section:", chopSectionTb};
  CHECK_NULLPTR_RETURN_NULLPTR(pDropSectionLabel);
  auto* pMaxPathLength = new (std::nothrow) QLabel{"Max path length:", chopSectionTb};
  CHECK_NULLPTR_RETURN_NULLPTR(pMaxPathLength);

  chopSectionTb->addWidget(pDropSectionLabel);
  chopSectionTb->addWidget(m_dropSectionLE);
  chopSectionTb->addSeparator();
  chopSectionTb->addWidget(pMaxPathLength);
  chopSectionTb->addWidget(m_maxPathLengthLE);
  chopSectionTb->addSeparator();
  chopSectionTb->addWidget(m_recursiveCB);
  return chopSectionTb;
}

void RenameWidget_LongPath::extraSubscribe() {
  connect(m_dropSectionLE, &QLineEdit::textEdited, this, &RenameWidget_LongPath::DropSectionChanged);
}

void RenameWidget_LongPath::InitExtraMemberWidget() {
  //
}

// only keep path length < 260
void RenameWidget_LongPath::FilterNames(FileOsWalker& walker) const {
  decltype(walker.relToNames) relToNames;
  decltype(walker.completeNames) completeNames;
  decltype(walker.suffixs) suffixs;
  decltype(walker.isFiles) isFiles;

  for (int i = 0; i < walker.relToNames.size(); ++i) {
    if (LongPathFinder::IsTooLong(walker.N                           //
                                  + walker.relToNames[i].size() + 1  //
                                  + walker.completeNames[i].size()   //
                                  + walker.suffixs[i].size())) {
      relToNames.append(walker.relToNames[i]);
      completeNames.append(walker.completeNames[i]);
      suffixs.append(walker.suffixs[i]);
      isFiles.append(walker.isFiles[i]);
    }
  }

  relToNames.swap(walker.relToNames);
  completeNames.swap(walker.completeNames);
  suffixs.swap(walker.suffixs);
  isFiles.swap(walker.isFiles);
}

QStringList RenameWidget_LongPath::RenameCore(const QStringList& replaceeList) {
  return m_lpf.GetNamesAfterSectionDropped(replaceeList);
}

void RenameWidget_LongPath::InitExtraCommonVariable() {
  windowTitleFormat = QString("Long Path name string | %1 item(s) under [%2]");
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/LONG_PATH_FINDER"));
}

void RenameWidget_LongPath::DropSectionChanged(const QString& newDropSectionStr) {
  bool isInt = false;
  int newDropSection = newDropSectionStr.toInt(&isInt);
  if (!isInt) {
    qWarning("invalid drop section index[%s]", qPrintable(newDropSectionStr));
    return;
  }
  m_lpf.SetDropSectionWhenTooLong(newDropSection);
  OnlyTriggerRenameCore();
}
