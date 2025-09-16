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

MD5Window::MD5Window(QWidget* parent) : QDialog{parent} {
  _ONLY_FIRST_8_BYTES = new (std::nothrow) QAction{QIcon{":img/ONLY_FIRST_8_BYTES"}, "Only First 8 Bytes", this};
  _ONLY_FIRST_8_BYTES->setCheckable(true);

  _ONLY_FIRST_16_BYTES = new (std::nothrow) QAction{QIcon{":img/ONLY_FIRST_16_BYTES"}, "Only First 16 Bytes", this};
  _ONLY_FIRST_16_BYTES->setCheckable(true);

  _ONLY_ENTIRE_FILE_BYTES = new (std::nothrow) QAction{QIcon{":img/ONLY_EVERY_BYTES"}, "Every Bytes", this};
  _ONLY_ENTIRE_FILE_BYTES->setCheckable(true);

  {
    using namespace BytesRangeTool;
    mBytesRangeIntAct.init({{_ONLY_FIRST_8_BYTES,     BytesRangeE::FIRST_8},      //
                            {_ONLY_FIRST_16_BYTES,    BytesRangeE::FIRST_16},     //
                            {_ONLY_ENTIRE_FILE_BYTES, BytesRangeE::ENTIRE_FILE}}, //
                           BytesRangeTool::DEFAULT_BYTE_RANGE, QActionGroup::ExclusionPolicy::Exclusive);
    mBytesRangeIntAct.setCheckedIfActionExist(BytesRangeTool::DEFAULT_BYTE_RANGE);
  }

  _MD5    = new (std::nothrow) QAction{"Md5", this};
  _MD5->setCheckable(true);
  _SHA1   = new (std::nothrow) QAction{"Sha1", this};
  _SHA1->setCheckable(true);
  _SHA256 = new (std::nothrow) QAction{"Sha256", this};
  _SHA256->setCheckable(true);
  _SHA512 = new (std::nothrow) QAction{"Sha512", this};
  _SHA512->setCheckable(true);

  {
    mHashAlgIntAct.init({{_MD5,    QCryptographicHash::Algorithm::Md5   },  //
                         {_SHA1,   QCryptographicHash::Algorithm::Sha1  },  //
                         {_SHA256, QCryptographicHash::Algorithm::Sha256},  //
                         {_SHA512, QCryptographicHash::Algorithm::Sha512}}, //
                        QCryptographicHash::Algorithm::Md5, QActionGroup::ExclusionPolicy::Exclusive);
    mHashAlgIntAct.setCheckedIfActionExist(QCryptographicHash::Algorithm::Md5);
  }

  m_md5InfoTB = new (std::nothrow) QToolBar{"Calculate MD5 Parms", this};
  m_md5InfoTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_md5InfoTB->addActions(mBytesRangeIntAct.getActionEnumAscendingList());
  m_md5InfoTB->addSeparator();
  m_md5InfoTB->addActions(mHashAlgIntAct.getActionEnumAscendingList());

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

int MD5Window::operator()(const QStringList& absPaths) {
  const BytesRangeTool::BytesRangeE curBytesRangeE = mBytesRangeIntAct.curVal();
  const int bytesRange = BytesRangeTool::toBytesValue(curBytesRangeE);

  const QCryptographicHash::Algorithm alg = mHashAlgIntAct.curVal();
  const QString bytesRangeAndAlgorithmStr{QString::asprintf("[%03d|%-03d]", bytesRange, (int)alg)};

  int newCalculatedFileCnt{0}, validFilesCnt{0};
  for (const QString& absPath : absPaths) {
    const QFileInfo fi{absPath};
    if (!fi.isFile()) {
      continue;
    }
    ++validFilesCnt;

    if (!mPathsList.contains(absPath)) {
      mPathsList.push_back(absPath);
    }

    QString md5;
    AbsFilePathBytesRangeHashAlgorithmKey key{absPath, bytesRange, alg};
    auto it = mAlreadyCalculatedHashMap.find(key);
    if (it != mAlreadyCalculatedHashMap.end()) { // no need calculate again
      md5 = it.value();
    } else {
      ++newCalculatedFileCnt;
      md5 = MD5Calculator::GetFileMD5(absPath, bytesRange, alg);
      mAlreadyCalculatedHashMap[key] = md5;
    }
    m_md5TextEdit->appendPlainText(md5 + ':' + bytesRangeAndAlgorithmStr + ':' + absPath);
  }
  m_md5TextEdit->appendPlainText(""); // add an empty line

  setWindowTitle(QString("MD5 | %1 file(s)").arg(validFilesCnt));
  LOG_OK_P("MD5 calculated ok", "+%d/%d file(s)", newCalculatedFileCnt, validFilesCnt);
  return newCalculatedFileCnt;
}

void MD5Window::dropEvent(QDropEvent* event) {
  const QMimeData* pMimedata = event->mimeData();
  if (pMimedata == nullptr) {
    event->ignore();
    return;
  }
  if (!pMimedata->hasUrls()) {
    event->ignore();
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
  connect(mBytesRangeIntAct.getActionGroup(), &QActionGroup::triggered, this, &MD5Window::Recalculate);
  connect(mHashAlgIntAct.getActionGroup(), &QActionGroup::triggered, this, &MD5Window::Recalculate);
}

void MD5Window::Recalculate() {
  operator()(mPathsList);
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
