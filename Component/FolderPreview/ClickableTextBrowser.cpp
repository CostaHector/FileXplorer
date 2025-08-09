#include "ClickableTextBrowser.h"
#include "public/PublicVariable.h"
#include "public/DisplayEnhancement.h"
#include "Tools/FileDescriptor/TableFields.h"
#include "Tools/FileDescriptor/FdBasedDb.h"
#include <QKeySequence>
#include <QInputDialog>
#include <QDesktopServices>
#include <QTextDocument>
#include <QTextCursor>
#include <QSqlRecord>
#include <QSqlField>

const QString ClickableTextBrowser::HTML_H1_TEMPLATE{R"(<a href="file:///%1">%2</a>)"};
const QString ClickableTextBrowser::HTML_IMG_TEMPLATE{R"(<a href="file:///%1"><img src="%1" alt="%2" width="%3"></a><br/>\n)"};
const QString ClickableTextBrowser::VID_LINK_TEMPLATE{R"(<a href="file:///%1">&#9654;%2</a>)"};
constexpr int ClickableTextBrowser::HTML_IMG_FIXED_WIDTH;

ClickableTextBrowser::ClickableTextBrowser(QWidget* parent) : QTextBrowser{parent} {
  setReadOnly(true);
  setOpenLinks(false);
  setOpenExternalLinks(true);

  m_menu = new QMenu{this};
  m_searchSelectionAction = m_menu->addAction(QIcon(":img/SEARCH"), "Search by selection");
  m_searchSelectionAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_F));
  m_editBeforeSearchAction = m_menu->addAction("Edit selection then Search");

  addAction(m_searchSelectionAction);

  connect(m_searchSelectionAction, &QAction::triggered, this, &ClickableTextBrowser::onSearchSelectionRequested);
  connect(m_editBeforeSearchAction, &QAction::triggered, this, &ClickableTextBrowser::onEditSelectionBeforeSearchRequested);
  connect(this, &QTextBrowser::anchorClicked, this, &ClickableTextBrowser::onAnchorClicked);
}

bool ClickableTextBrowser::onAnchorClicked(const QUrl& url) {
  if (!url.isLocalFile()) {
    return false;
  }
  return QDesktopServices::openUrl(url);
}

QString ClickableTextBrowser::GetStandardSearchKeyWords(QString text) {
  text = text.trimmed();
  static const QRegularExpression SPACES_AMPERSAND {R"(\s*&\s*)"};
  text.replace(JSON_RENAME_REGEX::AND_COMP, QChar{'&'});
  text.replace(',', QChar{'&'});
  text.replace(SPACES_AMPERSAND, QChar{'%'});
  QString stdKeyString;
  stdKeyString += '%';
  stdKeyString += text;
  stdKeyString += '%';
  return stdKeyString;
}

void ClickableTextBrowser::onSearchSelectionRequested() {
  QString searchKeyString = GetStandardSearchKeyWords(textCursor().selectedText());
  qDebug("Search:[%s]", qPrintable(searchKeyString));
  SearchAndAppendParagraphOfResult(searchKeyString);
}

void ClickableTextBrowser::onEditSelectionBeforeSearchRequested() {
  QStringList candidates;
  candidates.reserve(3);

  QString rawSelectedStr{textCursor().selectedText().trimmed()};
  candidates.push_back(GetStandardSearchKeyWords(rawSelectedStr));
  candidates.push_back(rawSelectedStr); // full match
  candidates.push_back('%' + rawSelectedStr + '%'); // contains

  bool bInputOk{false};
  const QString searchKeyString = //
      QInputDialog::getItem(this, //
                            "Search?", //
                            QString::number(candidates.size()) + " candidates as following...", //
                            candidates, 0, true, &bInputOk);
  if (!bInputOk) {
    qDebug("User cancel search by selection");
    return;
  }
  qDebug("Search:[%s]", qPrintable(searchKeyString));
  SearchAndAppendParagraphOfResult(searchKeyString);
}

QString ClickableTextBrowser::GetSearchResultParagraphDisplay(const QString& searchText) {
  FdBasedDb movieDbManager{SystemPath::VIDS_DATABASE, "EXIST_THEN_QRY_MOVIE_DB"};
  QString qryCmd = FdBasedDb::QUERY_KEY_INFO_TEMPLATE.arg(DB_TABLE::MOVIES, searchText);
  QList<QSqlRecord> records;
  if (!movieDbManager.QueryForTest(qryCmd, records)) {
    return QString{"<b>Query command failed</b>[%1]"}.arg(qryCmd);
  }
  if (records.isEmpty()) {
    return QString{"<b>Not in database</b>[%1]"}.arg(searchText);
  }

  QString searchResult;
  searchResult.reserve(512);
  searchResult += QString{"<b>%1 record(s) found</b> by key[%2]. They are:"}.arg(records.size()).arg(searchText);
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
