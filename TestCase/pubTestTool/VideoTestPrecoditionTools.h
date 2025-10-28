#ifndef VIDEOTESTPRECODITIONTOOLS_H
#define VIDEOTESTPRECODITIONTOOLS_H

#include <QString>
#include <QList>
#include <QByteArray>
namespace VideoTestPrecoditionTools {
constexpr char VID_DUR_GETTER_SAMPLE_PATH[] = TESTCASE_ROOT_PATH "/test/TestEnvVideosDurationGetter";
constexpr char TS_FILE_MERGER_SAMPLE_PATH[] = TESTCASE_ROOT_PATH "/test/TestEnvTSFilesMerger";

using SetDatabaseParmRetType = std::pair<bool, QString>;
SetDatabaseParmRetType setDupVidDbAbsFilePath(const QString& placeDbFileLocation);
SetDatabaseParmRetType setDupVidDbConnectionName(const QString& newConnectionNameUsed, int lineNo);

QByteArray GetVideoContentFFMPEGReadableOnly(int durationMs = 5000);
QByteArray CreateVideoContentNormal(const QString& videoGeneratedIn, int durationMs=5000, bool* bGenOk=nullptr);
QByteArray CreateVideoFile(const QString& videoGeneratedIn, const QList<quint32>& colorRGBs = {0xFF0000}, const QList<int>& durationMss = {10 * 1000},//
                           bool* bGenOk=nullptr, const int widthPx=480, const int heightPx=360);

bool IsFFmpegAvailable();
} // namespace VideoTestPrecoditionTools

#endif // VIDEOTESTPRECODITIONTOOLS_H
