#include "ClickableTextBrowser.h"
#include "BrowserActions.h"
#include "DataFormatter.h"
#include "FdBasedDb.h"
#include "CastBaseDb.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "JsonRenameRegex.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "TableFields.h"
#include "StringTool.h"
#include "StyleSheet.h"

#include <QKeySequence>
#include <QInputDialog>
#include <QDesktopServices>
#include <QTextDocument>
#include <QTextCursor>
#include <QSqlRecord>
#include <QSqlField>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

constexpr int ClickableTextBrowser::MIN_SINGLE_SEARCH_PATTERN_LEN;
constexpr int ClickableTextBrowser::MIN_EACH_KEYWORD_LEN;

ClickableTextBrowser::ClickableTextBrowser(QWidget* parent)//
  : QTextBrowser{parent}
{
  mCastVideosVisisble = Configuration().value(BrowserKey::CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS.name, BrowserKey::CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS.v).toBool();
  mCastImagesVisisble = Configuration().value(BrowserKey::CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES.name, BrowserKey::CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES.v).toBool();
  mCurIconSizeIndex = BrowserKey::CLICKABLE_TEXT_BROWSER_ICON_SIZE_INDEX.v.toInt();

  setOpenLinks(false);
  setOpenExternalLinks(true);

  double fontPointSize = Configuration().value(BrowserKey::CLICKABLE_TEXT_BROWSER_FONT_POINT_SIZE.name, BrowserKey::CLICKABLE_TEXT_BROWSER_FONT_POINT_SIZE.v).toDouble();
  auto curFont = font();
  curFont.setPointSizeF(fontPointSize);
  setFont(curFont);

  int iconSizeIndexHint = Configuration().value(BrowserKey::CLICKABLE_TEXT_BROWSER_ICON_SIZE_INDEX.name, BrowserKey::CLICKABLE_TEXT_BROWSER_ICON_SIZE_INDEX.v).toInt();
  mCurIconSizeIndex = std::max(0, std::min(iconSizeIndexHint, IMAGE_SIZE::ICON_SIZE_CANDIDATES_N-1)); // [0, WHEEL_CANDIDATES_N)
  mIconSize = IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconSizeIndex];

  auto& inst = BrowserActions::GetInst();
  mBrowserMenu = inst.GetSearchInDBMenu(this);
  mFloatingTb = inst.GetSearchInDBToolbar(this);

  setReadOnly(!inst.EDITOR_MODE->isChecked());

  connect(inst.SEARCH_CUR_TEXT, &QAction::triggered, this, &ClickableTextBrowser::onSearchSelectionReq);
  connect(inst.ADVANCED_TEXT_SEARCH, &QAction::triggered, this, &ClickableTextBrowser::onSearchSelectionAdvanceReq);
  connect(inst.SEARCH_MULTIPLE_TEXTS, &QAction::triggered, this, &ClickableTextBrowser::onSearchMultiSelectionReq);
  connect(inst.CLEAR_ALL_SELECTIONS, &QAction::triggered, this, &ClickableTextBrowser::ClearAllSelections);
  connect(inst.EDITOR_MODE, &QAction::triggered, this, [this](bool bEditable){ setReadOnly(!bEditable);});
  connect(inst.COPY_SELECTED_TEXT, &QAction::triggered, this, &ClickableTextBrowser::CopySelectedTextToClipboard);
  connect(inst.ADD_SELECTIONS_2_CAST_TABLE, &QAction::triggered, this, &ClickableTextBrowser::onAppendMultiSelectionToCastDbReq);
  connect(this, &QTextBrowser::anchorClicked, this, &ClickableTextBrowser::onAnchorClicked);

  AdjustButtonPosition();
}

ClickableTextBrowser::~ClickableTextBrowser() {
  Configuration().setValue(BrowserKey::CLICKABLE_TEXT_BROWSER_FONT_POINT_SIZE.name, font().pointSizeF());
  Configuration().setValue(BrowserKey::CLICKABLE_TEXT_BROWSER_ICON_SIZE_INDEX.name, mCurIconSizeIndex);
  Configuration().setValue(BrowserKey::CAST_PREVIEW_BROWSER_SHOW_RELATED_VIDEOS.name, mCastVideosVisisble);
  Configuration().setValue(BrowserKey::CAST_PREVIEW_BROWSER_SHOW_RELATED_IMAGES.name, mCastImagesVisisble);
}

void ClickableTextBrowser::wheelEvent(QWheelEvent *event) {
  if (event->modifiers() == Qt::ControlModifier) {
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
      int numSteps = numDegrees.y() / 15;
      int newSizeIndex = mCurIconSizeIndex + (numSteps > 0 ? 1 : -1);
      if (newSizeIndex < 0) {
        return;
      } else if (newSizeIndex >= IMAGE_SIZE::ICON_SIZE_CANDIDATES_N) {
        return;
      }
      mCurIconSizeIndex = newSizeIndex;
      mIconSize = IMAGE_SIZE::ICON_SIZE_CANDIDATES[mCurIconSizeIndex];
      LOG_OK_P("[Change] Icon size", "%d x %d", mIconSize.width(), mIconSize.height());
      QString htmlContents = toHtml();
      UpdateImagesSizeInHtmlSrc(htmlContents, mIconSize);
      setHtml(htmlContents);
      emit iconSizeChanged(mIconSize);
      event->accept();
      return;
    }
  }
  QTextBrowser::wheelEvent(event);
}

bool ClickableTextBrowser::onAnchorClicked(const QUrl& url) {
  if (url.isLocalFile()) {
    return QDesktopServices::openUrl(url);
  }
  bool hideOrShowRelated{false};
  if (url.toString() == "hideRelatedVideos") {
    hideOrShowRelated = true;
    mCastVideosVisisble = !mCastVideosVisisble;
  } else if (url.toString() == "hideRelatedImages") {
    hideOrShowRelated = true;
    mCastImagesVisisble = !mCastImagesVisisble;
  }
  if (hideOrShowRelated) {
    UpdateHtmlContents();
  }
  return true;
}

QString ClickableTextBrowser::FormatSearchSentence(QString text) {
  text = text.trimmed();
  if (text.isEmpty()) {
    return "%";
  }
  static const QRegularExpression SPACES_AMPERSAND {R"(\s*&\s*)"};
  text.replace(JSON_RENAME_REGEX::AND_COMP, QChar{'&'});
  text.replace(',', QChar{'&'});
  text.replace(SPACES_AMPERSAND, QChar{'%'});
  return '%' + text + '%';
}

void ClickableTextBrowser::onSearchSelectionReq() {
  const QString& searchKeyString {FormatSearchSentence(GetCurrentSelectedText())};
  if (searchKeyString.size() < MIN_SINGLE_SEARCH_PATTERN_LEN) {
    LOG_WARN_NP("Skip search, too short searchText:", searchKeyString);
    return;
  }
  LOG_D("Search:[%s]", qPrintable(searchKeyString));
  SearchAndAppendParagraphOfResult(FdBasedDb::WHERE_NAME_CORE_TEMPLATE.arg(searchKeyString));
}

void ClickableTextBrowser::onSearchSelectionAdvanceReq() {
  QStringList candidates;
  candidates.reserve(3);

  QString rawSelectedStr{GetCurrentSelectedText().trimmed()};
  candidates.push_back(FormatSearchSentence(rawSelectedStr));
  candidates.push_back(rawSelectedStr); // full match
  candidates.push_back('%' + rawSelectedStr + '%'); // contains

  bool bInputOk{false};
  const QString searchKeyString = //
      QInputDialog::getItem(this, //
                            "Search from below text?", //
                            "There are " + QString::number(candidates.size()) + "candidates you can use as following...", //
                            candidates, 0, true, &bInputOk);
  if (!bInputOk) {
    LOG_D("User cancel search by selection");
    return;
  }
  if (searchKeyString.size() < MIN_SINGLE_SEARCH_PATTERN_LEN) {
    LOG_WARN_P("[Skip search] too short", "searchText[%s] should >= %d chars]", qPrintable(searchKeyString), MIN_SINGLE_SEARCH_PATTERN_LEN);
    return;
  }
  LOG_D("Search:[%s]", qPrintable(searchKeyString));
  SearchAndAppendParagraphOfResult(FdBasedDb::WHERE_NAME_CORE_TEMPLATE.arg(searchKeyString));
}

void ClickableTextBrowser::onSearchMultiSelectionReq() {
  const QStringList keywords = GetSelectedTexts();
  if (keywords.isEmpty()) {
    return;
  }
  bool bNeedSearchDb{false};
  const QString whereClause = BuildMultiKeywordLikeCondition(keywords, bNeedSearchDb);
  if (!bNeedSearchDb) {
    LOG_WARN_P("[Skip search] too short", "Max Sub-Conditon keyword of whereClause[%s] should >= %d chars]",
               qPrintable(whereClause), MIN_EACH_KEYWORD_LEN);
    return;
  }
  LOG_D("Search:[%s]", qPrintable(whereClause));
  SearchAndAppendParagraphOfResult(whereClause);
}

int ClickableTextBrowser::onAppendMultiSelectionToCastDbReq() {
  CastBaseDb castDb{SystemPath::PEFORMERS_DATABASE(), "CAST_CONNECTION"};
  const auto db = castDb.GetDb();
  if (!db.tables().contains(DB_TABLE::PERFORMERS)) {
    LOG_ERR_NP("[Abort] Cast Table not exist", DB_TABLE::PERFORMERS);
    return -1;
  }

  QStringList keywords = GetSelectedTexts();
  StringTool::SearchHistoryListProc(keywords);
  const QString& rawStringFromSelection {keywords.join(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR)};
  bool ok = false;
  const QString& perfsText =                             //
      QInputDialog::getMultiLineText(this, "Edit 'Casts(, aka)'",  //
                                     "Example:\n Guardiola, Pep\nHuge Jackman, Wolverine",
                                     rawStringFromSelection, &ok);
  if (!ok) {
    LOG_OK_NP("[skip] User cancel append", "return");
    return 0;
  }
  const int insertOrUpdateCnt = castDb.AppendCastFromMultiLineInput(perfsText);
  if (insertOrUpdateCnt < FD_OK) {
    LOG_ERR_P("[Failed] Cast Inserted/Update", "errorCode: %d", insertOrUpdateCnt);
    return insertOrUpdateCnt;
  }
  LOG_OK_P("[Ok]Cast Inserted/Update", "%d casts added as follows:%s",
           insertOrUpdateCnt, qPrintable(perfsText));
  return insertOrUpdateCnt;
}


QString ClickableTextBrowser::GetSearchResultParagraphDisplay(const QString& whereText) {
  FdBasedDb movieDbManager{SystemPath::VIDS_DATABASE(), "EXIST_THEN_QRY_MOVIE_DB"};
  QString qryCmd = FdBasedDb::QUERY_KEY_INFO_TEMPLATE.arg(DB_TABLE::MOVIES, whereText);
  QList<QSqlRecord> records;
  if (!movieDbManager.QueryForTest(qryCmd, records)) {
    return QString{"<b>Query command failed</b>[%1]"}.arg(qryCmd);
  }
  if (records.isEmpty()) {
    return QString{"<b>Not in database</b>[%1]"}.arg(whereText);
  }

  QString searchResult;
  searchResult.reserve(512);
  searchResult += QString{"<b>%1 record(s) found</b> by key[%2]. They are:"}.arg(records.size()).arg(whereText);
  searchResult += "<table border='1' cellpadding='4' style='border-collapse: collapse;'>";
  searchResult += "<thead><tr><th>Size</th><th>Name</th><th>Path</th></tr></thead>";
  searchResult += "<tbody>";

  static constexpr int NAME_FILED_IND {(int)FdBasedDb::QUERY_KEY_INFO_FIELED::Name};
  static constexpr int SIZE_FILED_IND {(int)FdBasedDb::QUERY_KEY_INFO_FIELED::Size};
  static constexpr int PREPATH_R_FILED_IND {(int)FdBasedDb::QUERY_KEY_INFO_FIELED::PrePathRight};

  for (const QSqlRecord& record: records) {
    const qint64 sz = record.field(SIZE_FILED_IND).value().toLongLong();
    searchResult += "<tr>";
    searchResult += QString{"<td>%1</td>"}.arg(DataFormatter::formatFileSizeGMKB(sz));
    searchResult += QString{"<td>%1</td>"}.arg(record.field(NAME_FILED_IND).value().toString());
    searchResult += QString{"<td>%1</td>"}.arg(record.field(PREPATH_R_FILED_IND).value().toString());
    searchResult += "</tr>";
  }

  searchResult += "</tbody></table>";
  return searchResult;
}

void ClickableTextBrowser::mouseDoubleClickEvent(QMouseEvent *e) {
  if (e->button() == Qt::LeftButton) {
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    AppendASelection(cursor);
  }
  QTextBrowser::mouseDoubleClickEvent(e);
}

void ClickableTextBrowser::mousePressEvent(QMouseEvent *e) {
  if (e->button() == Qt::LeftButton) {
    mbDragging = true;
    mDraggingStartPos = e->pos();
  }
  QTextBrowser::mousePressEvent(e);
}

void ClickableTextBrowser::mouseMoveEvent(QMouseEvent *e) {
  if (!mbDragging) {
    if (e->buttons() & Qt::LeftButton) {
      const QString& curString = GetCurrentSelectedText();
      if (!curString.isEmpty()) {
        QMimeData *mimeData = new (std::nothrow) QMimeData;
        CHECK_NULLPTR_RETURN_VOID(mimeData);
        mimeData->setText(curString);
        QDrag *drag = new (std::nothrow) QDrag{this};
        CHECK_NULLPTR_RETURN_VOID(drag);
        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction);
      }
    }
  }
  QTextBrowser::mouseMoveEvent(e);
}

void ClickableTextBrowser::mouseReleaseEvent(QMouseEvent *e) {
  if (e->button() == Qt::LeftButton && mbDragging) {
    mbDragging = false;
    AppendASelection(textCursor());
  }
  QTextBrowser::mouseReleaseEvent(e);
}

void ClickableTextBrowser::AppendASelection(const QTextCursor &cursor) {
  if (!cursor.hasSelection()) {
    return; // ignore empty selection
  }
  QTextEdit::ExtraSelection extra;
  extra.cursor = cursor;
  extra.format.setBackground(Qt::yellow);
  mMultiSelections.append(extra);
  setExtraSelections(mMultiSelections);
}

void ClickableTextBrowser::ClearAllSelections() {
  mMultiSelections.clear();
  setExtraSelections(mMultiSelections);
}

void ClickableTextBrowser::SearchAndAppendParagraphOfResult(const QString& searchText) {
  QTextDocument *doc = document();
  QTextCursor cursor{doc};
  cursor.movePosition(QTextCursor::End);

  QString para;
  para.reserve(512);
  para += "<br/>";
  para += R"(<font size="+2">)";
  para += GetSearchResultParagraphDisplay(searchText);
  para += "</font>";
  cursor.insertHtml(para);
}

QString ClickableTextBrowser::BuildMultiKeywordLikeCondition(const QStringList &keywords, bool& pNeedSearchDb) {
  pNeedSearchDb = false;

  QStringList conditions;
  conditions.reserve(keywords.size());

  QString searchText;
  searchText.reserve(20);
  for (const QString &keyword : keywords) {
    if (keyword.isEmpty()) {
      continue;
    }
    if (keyword.size() >= MIN_EACH_KEYWORD_LEN) {
      pNeedSearchDb = true;
    }
    searchText.clear();
    searchText += '%';
    searchText += keyword;
    searchText += '%';
    conditions.push_back(FdBasedDb::WHERE_NAME_CORE_TEMPLATE.arg(searchText));
  }
  if (conditions.isEmpty()) {
    return "1=1";
  }
  return conditions.join(" AND ");
}

#include <QRegularExpression>
QString& ClickableTextBrowser::UpdateImagesSizeInHtmlSrc(QString& htmlSrc, const QSize& newSize) {
  static const QRegularExpression widthFixedRepRegex("<img(.*?)width=\"(\\d{0,4})\"(.*?)>", QRegularExpression::CaseInsensitiveOption);
  static const QRegularExpression heightFixedRepRegex("<img(.*?)height=\"(\\d{0,4})\"(.*?)>", QRegularExpression::CaseInsensitiveOption);
  htmlSrc.replace(widthFixedRepRegex, QString(R"(<img\1width="%1"\3>)").arg(newSize.width()));
  htmlSrc.replace(heightFixedRepRegex, QString(R"(<img\1height="%1"\3>)").arg(newSize.height()));
  return htmlSrc;
}

void ClickableTextBrowser::CopySelectedTextToClipboard() const {
  QClipboard* pClipboard = QApplication::clipboard();
  if (pClipboard == nullptr) {
    LOG_ERR_NP("Copy failed", "pClipboard is nullptr");
    return;
  }
  pClipboard->setText(GetCurrentSelectedText());
}
