#include "VideoTestPrecoditionTools.h"
#include "VidDupTabFields.h"
#include "PublicTool.h"
#include <QProcess>
#include <QBuffer>

namespace VideoTestPrecoditionTools {
SetDatabaseParmRetType setDupVidDbAbsFilePath(const QString& placeDbFileLocation) {
  QString actualDupDbAbsFilePath = VidDupHelper::DupVidDbAbsFilePathInTestCase(placeDbFileLocation);
  if (!actualDupDbAbsFilePath.startsWith(placeDbFileLocation, Qt::CaseSensitivity::CaseSensitive)) {
    return {false, actualDupDbAbsFilePath};
  }
  if (!actualDupDbAbsFilePath.endsWith(".db", Qt::CaseSensitivity::CaseSensitive)) {
    return {false, actualDupDbAbsFilePath};
  }
  return {true, actualDupDbAbsFilePath};
}
SetDatabaseParmRetType setDupVidDbConnectionName(const QString& newConnectionNameUsed, int lineNo) {
  const QString expect_connectionName = newConnectionNameUsed + QString::number(lineNo);
  const QString actualDupDbConnectionName = VidDupHelper::DupVidDbConnectionNameInTestCase(expect_connectionName);
  if (actualDupDbConnectionName != expect_connectionName) {
    return {false, actualDupDbConnectionName};
  }
  return {true, actualDupDbConnectionName};
}

bool IsFFmpegAvailable() {
  QProcess ffmpeg;
  ffmpeg.start("ffmpeg", {"-version"});
  return ffmpeg.waitForStarted() && ffmpeg.waitForFinished();
}

QByteArray GetVideoContentFFMPEGReadableOnly(int durationMs) {
  // 创建一个非常简单的视频容器（仅用于测试）
  QByteArray videoData;
  QBuffer buffer(&videoData);
  buffer.open(QIODevice::WriteOnly);

  // 简单的视频容器头部（模拟5秒视频）
  buffer.write("RIFF");
  buffer.write(QByteArray::fromHex("00000000"));  // 文件大小占位符
  buffer.write("AVI ");

  // 添加时长信息
  buffer.write("LIST");
  buffer.write(QByteArray::fromHex("00000000"));  // 列表大小占位符
  buffer.write("hdrl");
  buffer.write("avih");
  buffer.write(QByteArray::fromHex("38000000"));                               // avih块大小
  buffer.write(QByteArray::number(durationMs).rightJustified(8, '\0', true));  // 时长

  // 填充文件大小
  qint64 fileSize = buffer.size();
  buffer.seek(4);
  buffer.write(QByteArray::number(fileSize - 8).rightJustified(4, '\0', true));

  buffer.close();
  return videoData;
}

QByteArray CreateVideoContentNormal(const QString& videoGeneratedIn, int durationMs, bool* bGenOk) {
  int durationSeconds = durationMs / 1000;
  // 使用 FFmpeg 生成视频
  QProcess ffmpeg;
  QStringList args;
  args << "-y"  // 覆盖输出文件
       << "-f"
       << "lavfi"
       << "-i" << QString("color=c=red:s=100x80:d=%1").arg(durationSeconds) << "-c:v"
       << "libx264"
       << "-t" << QString::number(durationSeconds) << videoGeneratedIn;

  ffmpeg.start("ffmpeg", args);
  ffmpeg.waitForFinished();

  // 读取生成的视频文件
  return FileTool::ByteArrayReader(videoGeneratedIn, bGenOk);
}
}  // namespace VideoTestPrecoditionTools
