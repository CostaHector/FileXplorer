#include "MD5Window.h"
#include "NotificatorMacro.h"
#include "MD5Calculator.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QIcon>
#include <QMimeData>

MD5Window::MD5Window(QWidget* parent)
  : QDialog{parent}  //
{
  _ONLY_FIRST_8_BYTES = new (std::nothrow) QAction{QIcon{":img/ONLY_FIRST_8_BYTES"}, "Only First 8 Bytes", this};
  _ONLY_FIRST_8_BYTES->setCheckable(true);
  _ONLY_FIRST_8_BYTES->setChecked(true);

  _ONLY_FIRST_16_BYTES = new (std::nothrow) QAction{QIcon{":img/ONLY_FIRST_16_BYTES"}, "Only First 16 Bytes", this};
  _ONLY_FIRST_16_BYTES->setCheckable(true);

  _ONLY_EVERY_BYTES = new (std::nothrow) QAction{QIcon{":img/ONLY_EVERY_BYTES"}, "Only Every Bytes", this};
  _ONLY_EVERY_BYTES->setCheckable(true);

  _CALC_BYTES_RANGE = new (std::nothrow) QActionGroup{this};
  _CALC_BYTES_RANGE->addAction(_ONLY_FIRST_8_BYTES);
  _CALC_BYTES_RANGE->addAction(_ONLY_FIRST_16_BYTES);
  _CALC_BYTES_RANGE->addAction(_ONLY_EVERY_BYTES);
  _CALC_BYTES_RANGE->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  m_md5InfoTB = new (std::nothrow) QToolBar{"Calculate MD5 Parms", this};
  m_md5InfoTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_md5InfoTB->addActions(_CALC_BYTES_RANGE->actions());

  m_md5TextEdit = new (std::nothrow) QPlainTextEdit{this};

  mMainLayout = new (std::nothrow) QVBoxLayout(this);
  mMainLayout->addWidget(m_md5InfoTB);
  mMainLayout->addWidget(m_md5TextEdit);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
  setWindowTitle("MD5 Window | Drop files here");
  setWindowIcon(QIcon(":img/MD5_FILE_IDENTIFIER_PATH"));

  m_md5TextEdit->setAcceptDrops(false);
  setAcceptDrops(true);

  ReadSetting();
  subscribe();
}

void MD5Window::ReadSetting() {
  if (Configuration().contains("MD5WindowGeometry")) {
    restoreGeometry(Configuration().value("MD5WindowGeometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  m_md5TextEdit->setFont(StyleSheet::TEXT_EDIT_FONT);
}

void MD5Window::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void MD5Window::closeEvent(QCloseEvent* event) {
  Configuration().setValue("MD5WindowGeometry", saveGeometry());
  QDialog::closeEvent(event);
}

int MD5Window::operator()(const QStringList& absPaths) {
  const int bytesRange = GetBytesRange();
  const QString bytesRangeStr{QString("%1").arg(bytesRange, 3, 10, QChar{' '})};
  int fileCnt{0};
  for (const QString& absPath : absPaths) {
    const QFileInfo fi{absPath};
    if (!fi.isFile()) {
      continue;
    }
    if (!mPathsSet.contains(absPath)) {
      mPathsList.append(absPath);
      mPathsSet.insert(absPath);
    }
    ++fileCnt;
    const QString md5{MD5Calculator::GetFileMD5(absPath, bytesRange)};
    m_md5TextEdit->appendPlainText(md5 + ':' + bytesRangeStr + ':' + absPath);
  }

  setWindowTitle(QString("MD5 | %1 file(s)").arg(fileCnt));
  LOG_OK_P("MD5 calculated ok", "%d file(s)", fileCnt);
  return fileCnt;
}

void MD5Window::dropEvent(QDropEvent* event) {
  const QMimeData* pMimedata = event->mimeData();
  if (pMimedata == nullptr) {
    event->ignore();
    return;
  }
  if (!pMimedata->hasUrls()) {
    QDialog::dropEvent(event);
    return;
  }
  const QList<QUrl>& urls = pMimedata->urls();
  QStringList absPaths;
  absPaths.reserve(urls.size());
  for (const QUrl& url : urls) {
    absPaths.append(url.toLocalFile());
  }
  operator()(absPaths);
  event->accept();
}

void MD5Window::dragEnterEvent(QDragEnterEvent* event) {
  const QMimeData* pMimedata = event->mimeData();
  if (pMimedata == nullptr) {
    event->ignore();
    return;
  }
  if (!pMimedata->hasUrls()) {
    event->accept();
    return;
  }
  QDialog::dragEnterEvent(event);
}

void MD5Window::subscribe() {
  connect(_CALC_BYTES_RANGE, &QActionGroup::triggered, this, &MD5Window::Recalculate);
}

int MD5Window::GetBytesRange() const {
  if (_CALC_BYTES_RANGE == nullptr)  {
    return -1;
  }
  auto* pAct = _CALC_BYTES_RANGE->checkedAction();
  if (pAct == _ONLY_FIRST_8_BYTES) {
    return 16;
  } else if (pAct == _ONLY_FIRST_16_BYTES) {
    return 64;
  } else {
    return -1;
  }
}

void MD5Window::Recalculate() {
  operator()(mPathsList);
}
