#include "MD5Window.h"
#include "NotificatorMacro.h"
#include "MD5Calculator.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include <QMenu>
#include <QFileInfo>
#include <QIcon>
#include <QMimeData>
#include <QMetaEnum>

QString algorithmToString(QCryptographicHash::Algorithm alg) {
  QMetaEnum metaEnum = QMetaEnum::fromType<QCryptographicHash::Algorithm>();  // 获取元枚举
  const char* name = metaEnum.valueToKey(static_cast<int>(alg));
  if (name == nullptr) {
    return QString::asprintf("Unknown[%d]", alg);
  }
  return QString::asprintf("%s[%d]", name, alg);
}
// each cell 16kB
// dimension: 8*8
// total = 16 * 64 = 1024kB
MD5Window::MD5Window(QWidget* parent) : QDialog{parent} {
  // 优化后的 QAction 创建
  _ONLY_FIRST_16_BYTES = new (std::nothrow) QAction{QIcon{":img/FILE_FIRST_16_BYTES"}, tr("First 16 Bytes"), this};
  _ONLY_FIRST_16_BYTES->setCheckable(true);
  _ONLY_FIRST_16_BYTES->setToolTip(
      "Use only the first 16 bytes of the file to calculate the hash.\n"
      "Quick method that balances speed and uniqueness.");
  _ONLY_FIRST_16_BYTES->setStatusTip("Calculate hash using first 16 bytes of the file");

  _SAMPLED_128_KB = new (std::nothrow) QAction{QIcon(":img/FILE_RATIO_128KB"), tr("Sample 128 kBytes"), this};
  _SAMPLED_128_KB->setCheckable(true);
  _SAMPLED_128_KB->setToolTip(
      "Sample 8 equally spaced 16kB chunks (total 128kB) to calculate hash.\n"
      "Suitable for medium-sized files, balances speed and accuracy.");
  _SAMPLED_128_KB->setStatusTip("Sample 16 chunks of 8192 bytes for hash calculation");

  _SAMPLED_512_KB = new (std::nothrow) QAction{QIcon(":img/FILE_RATIO_512KB"), tr("Sample 512 kBytes (Balanced)"), this};
  _SAMPLED_512_KB->setCheckable(true);
  _SAMPLED_512_KB->setToolTip(
      "Sample 16 equally spaced 32kB chunks (total 512kB) to calculate hash.\n"
      "Recommended for most files, provides good accuracy with reasonable speed.");
  _SAMPLED_512_KB->setStatusTip("Sample 32 chunks of 8192 bytes for balanced hash calculation");

  _ENTIRE_FILE_BYTES = new (std::nothrow) QAction{QIcon{":img/ENTIRE_FILE_BYTES"}, tr("Entire File"), this};
  _ENTIRE_FILE_BYTES->setCheckable(true);
  _ENTIRE_FILE_BYTES->setToolTip(
      "Use the entire file content to calculate the hash.\n"
      "Slowest but most accurate method, ensures zero collision risk.");
  _ENTIRE_FILE_BYTES->setStatusTip("Calculate hash using the entire file content");

  {
    using namespace BytesRangeTool;
    mBytesRangeIntAct.init({{_ONLY_FIRST_16_BYTES, BytesRangeE::FIRST_16_BYTES},  //
                            {_SAMPLED_128_KB, BytesRangeE::SAMPLED_128_KB},           //
                            {_SAMPLED_512_KB, BytesRangeE::SAMPLED_512_KB},           //
                            {_ENTIRE_FILE_BYTES, BytesRangeE::ENTIRE_FILE}},      //
                           BytesRangeTool::DEFAULT_BYTE_RANGE, QActionGroup::ExclusionPolicy::Exclusive);
    mBytesRangeIntAct.setCheckedIfActionExist(BytesRangeTool::DEFAULT_BYTE_RANGE);
  }

  _MD5 = new (std::nothrow) QAction{algorithmToString(QCryptographicHash::Algorithm::Md5), this};
  _MD5->setCheckable(true);
  _SHA1 = new (std::nothrow) QAction{algorithmToString(QCryptographicHash::Algorithm::Sha1), this};
  _SHA1->setCheckable(true);
  _SHA256 = new (std::nothrow) QAction{algorithmToString(QCryptographicHash::Algorithm::Sha256), this};
  _SHA256->setCheckable(true);
  _SHA512 = new (std::nothrow) QAction{algorithmToString(QCryptographicHash::Algorithm::Sha512), this};
  _SHA512->setCheckable(true);
  {
    mHashAlgIntAct.init({{_MD5, QCryptographicHash::Algorithm::Md5},         //
                         {_SHA1, QCryptographicHash::Algorithm::Sha1},       //
                         {_SHA256, QCryptographicHash::Algorithm::Sha256},   //
                         {_SHA512, QCryptographicHash::Algorithm::Sha512}},  //
                        QCryptographicHash::Algorithm::Md5, QActionGroup::ExclusionPolicy::Exclusive);
    mHashAlgIntAct.setCheckedIfActionExist(QCryptographicHash::Algorithm::Md5);
  }
  auto* pHashAlgorithmMenu = new (std::nothrow) QMenu{"Algorithm Menu", this};
  pHashAlgorithmMenu->addActions(mHashAlgIntAct.getActionEnumAscendingList());
  _HASH_ALGORITHM_TOOLBUTTON = new (std::nothrow) QToolButton{this};
  _HASH_ALGORITHM_TOOLBUTTON->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  _HASH_ALGORITHM_TOOLBUTTON->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
  _HASH_ALGORITHM_TOOLBUTTON->setIcon(QIcon{":img/HASH_ALGORITHM"});
  _HASH_ALGORITHM_TOOLBUTTON->setText("Hash Algorithm");
  _HASH_ALGORITHM_TOOLBUTTON->setMenu(pHashAlgorithmMenu);

  m_md5InfoTB = new (std::nothrow) QToolBar{"Calculate MD5 Parms", this};
  m_md5InfoTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_md5InfoTB->addWidget(_HASH_ALGORITHM_TOOLBUTTON);
  m_md5InfoTB->addSeparator();
  m_md5InfoTB->addActions(mBytesRangeIntAct.getActionEnumAscendingList());

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
  const QString bytesRangeAndAlgorithmStr{QString::asprintf("[%s|%03d]", BytesRangeTool::c_str(curBytesRangeE), (int)alg)};

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

    QByteArray md5;
    AbsFilePathBytesRangeHashAlgorithmKey key{absPath, bytesRange, alg};
    auto it = mAlreadyCalculatedHashMap.find(key);
    if (it != mAlreadyCalculatedHashMap.end()) {  // no need calculate again
      md5 = it.value();
    } else {
      ++newCalculatedFileCnt;
      md5 = MD5Calculator::GetFileMD5(absPath, curBytesRangeE, alg);
      mAlreadyCalculatedHashMap[key] = md5;
    }
    m_md5TextEdit->appendPlainText(md5 + ':' + bytesRangeAndAlgorithmStr + ':' + absPath);
  }
  m_md5TextEdit->appendPlainText("");  // add an empty line

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
    event->ignore();
    return;
  }
  event->accept();
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
