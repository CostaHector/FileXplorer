#include "ClickableTextBrowser.h"
#include "BrowserActions.h"
#include "PublicVariable.h"
#include "DisplayEnhancement.h"
#include "TableFields.h"
#include "FdBasedDb.h"
#include "Notificator.h"
#include <QKeySequence>
#include <QInputDialog>
#include <QDesktopServices>
#include <QTextDocument>
#include <QTextCursor>
#include <QSqlRecord>
#include <QSqlField>
#include <QDebug>

const QString ClickableTextBrowser::HTML_H1_TEMPLATE{R"(<a href="file:///%1">%2</a>)"};
const QString ClickableTextBrowser::HTML_IMG_TEMPLATE{R"(<a href="file:///%1"><img src="%1" alt="%2" width="%3"></a><br/>\n)"};
const QString ClickableTextBrowser::VID_LINK_TEMPLATE{R"(<a href="file:///%1">&#9654;%2</a>)"};
constexpr int ClickableTextBrowser::HTML_IMG_FIXED_WIDTH;
constexpr int ClickableTextBrowser::MIN_KEYWORD_LEN;

ClickableTextBrowser::ClickableTextBrowser(QWidget* parent) : QTextBrowser{parent} {
  setOpenLinks(false);
  setOpenExternalLinks(true);

  auto& inst = BrowserActions::GetInst();
  mBrowserMenu = inst.GetSearchInDBMenu(this);
  mFloatingTb = inst.GetSearchInDBToolbar(this);

  setReadOnly(!inst.EDITOR_MODE->isChecked());

  connect(inst.SEARCH_CUR_TEXT, &QAction::triggered, this, &ClickableTextBrowser::onSearchSelectionReq);
  connect(inst.ADVANCED_TEXT_SEARCH, &QAction::triggered, this, &ClickableTextBrowser::onSearchSelectionAdvanceReq);
  connect(inst.SEARCH_MULTIPLE_TEXTS, &QAction::triggered, this, &ClickableTextBrowser::onSearchMultiSelectionReq);
  connect(inst.CLEAR_ALL_SELECTIONS, &QAction::triggered, this, &ClickableTextBrowser::ClearAllSelections);
  connect(inst.EDITOR_MODE, &QAction::triggered, this, [this](bool bEditable){ setReadOnly(!bEditable);});
  connect(this, &QTextBrowser::anchorClicked, this, &ClickableTextBrowser::onAnchorClicked);

  AdjustButtonPosition();
}

bool ClickableTextBrowser::onAnchorClicked(const QUrl& url) {
  if (!url.isLocalFile()) {
    return false;
  }
  return QDesktopServices::openUrl(url);
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
  QString searchKeyString = FormatSearchSentence(GetCurrentSelectedText());
  if (searchKeyString.size() < MIN_KEYWORD_LEN) {
    LOG_WARN("Skip search, too short searchText:", searchKeyString);
    return;
  }
  qDebug("Search:[%s]", qPrintable(searchKeyString));
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
    qDebug("User cancel search by selection");
    return;
  }
  if (searchKeyString.size() < MIN_KEYWORD_LEN) {
    LOG_WARN("Skip search, too short searchText:", searchKeyString);
    return;
  }
  qDebug("Search:[%s]", qPrintable(searchKeyString));
  SearchAndAppendParagraphOfResult(FdBasedDb::WHERE_NAME_CORE_TEMPLATE.arg(searchKeyString));
}

void ClickableTextBrowser::onSearchMultiSelectionReq() {
  const QStringList keywords = GetSelectedTexts();
  if (keywords.isEmpty()) {
    LOG_WARN("Skip search", "Nothing selected");
    return;
  }
  bool bNeedSearchDb{false};
  const QString whereClause = BuildMultiKeywordLikeCondition(keywords, bNeedSearchDb);
  if (!bNeedSearchDb) {
    LOG_WARN("Skip search, too short whereClause:", whereClause);
    return;
  }
  qDebug("Search:[%s]", qPrintable(whereClause));
  SearchAndAppendParagraphOfResult(whereClause);
}

QString ClickableTextBrowser::GetSearchResultParagraphDisplay(const QString& whereText) {
  FdBasedDb movieDbManager{SystemPath::VIDS_DATABASE, "EXIST_THEN_QRY_MOVIE_DB"};
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

  for (const auto& record: records) {
    const qint64 sz = record.field(SIZE_FILED_IND).value().toLongLong();
    searchResult += "<tr>";
    searchResult += QString{"<td>%1</td>"}.arg(FILE_PROPERTY_DSP::sizeToHumanReadFriendly(sz));
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
  if (mbDragging) {
    QTextCursor cursor = textCursor();
    cursor.setPosition(cursorForPosition(mDraggingStartPos).position());
    cursor.setPosition(cursorForPosition(e->pos()).position(), QTextCursor::KeepAnchor);
    setTextCursor(cursor);
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
    searchText.clear();
    searchText += '%';
    searchText += keyword;
    searchText += '%';
    if (searchText.size() >= MIN_KEYWORD_LEN) {
      pNeedSearchDb = true;
    }
    conditions.push_back(FdBasedDb::WHERE_NAME_CORE_TEMPLATE.arg(searchText));
  }
  if (conditions.isEmpty()) {
    return "1=1";
  }
  return conditions.join(" AND ");
}
