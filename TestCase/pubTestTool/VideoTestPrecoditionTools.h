#ifndef VIDEOTESTPRECODITIONTOOLS_H
#define VIDEOTESTPRECODITIONTOOLS_H

#include <QString>
#include <QByteArray>
namespace VideoTestPrecoditionTools {
constexpr char VID_DUR_GETTER_SAMPLE_PATH[] = TESTCASE_ROOT_PATH "/test/TestEnvVideosDurationGetter";
constexpr char TS_FILE_MERGER_SAMPLE_PATH[] = TESTCASE_ROOT_PATH "/test/TestEnvTSFilesMerger";

using SetDatabaseParmRetType = std::pair<bool, QString>;
SetDatabaseParmRetType setDupVidDbAbsFilePath(const QString& placeDbFileLocation);
SetDatabaseParmRetType setDupVidDbConnectionName(const QString& newConnectionNameUsed, int lineNo);

QByteArray GetVideoContentFFMPEGReadableOnly(int durationMs = 5000);
QByteArray CreateVideoContentNormal(const QString& videoGeneratedIn, int durationMs=5000, bool* bGenOk=nullptr);
bool IsFFmpegAvailable();
} // namespace VideoTestPrecoditionTools

#endif // VIDEOTESTPRECODITIONTOOLS_H
