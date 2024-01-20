#include "MP4DurationGetter.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QFileInfo>

// QTC_TEMP
char* MP4DurationGetter::videoinfo_mem(char* haystack, unsigned int sizehaystack, char* needle, unsigned int sizeneedle) {
  int i = 0;
  int end = sizehaystack - sizeneedle;
  for (i = 0; i < end; ++i) {
    if (memcmp(haystack + i, needle, sizeneedle) == 0) {
      return haystack + i;
    }
  }
  return NULL;
}

void* MP4DurationGetter::videoinfo_find(char* filename, void* find, int size, int resultSize) {
  FILE* fp = NULL;
  char* buffer = NULL;
  char* result = NULL;
  char* pos = NULL;
  unsigned int bufferSize = 2048;
  double filesize = 0;
  double split = 16;
  double splitsize = 0;
  double start = 0;
  double end = 0;
  double i = 0;
  unsigned int read = 0;

  if (resultSize > bufferSize) {
    resultSize = bufferSize;
  }

  buffer = (char*)malloc(bufferSize);
  if (buffer == NULL) {
    return NULL;
  }

  fp = fopen(filename, "rb");
  if (fp == NULL) {
    free(buffer);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  filesize = ftell(fp);
  rewind(fp);

  split = ceil(filesize / 100000);
  splitsize = ceil(filesize / split);

  for (i = split - 1; i >= 0; --i) {
    start = (i * splitsize);
    end = start + splitsize;
    fseek(fp, start, SEEK_SET);

    while ((read = fread(buffer, 1, bufferSize, fp)) != 0) {
      if ((pos = videoinfo_mem(buffer, bufferSize, (char*)find, size)) != NULL) {
        result = (char*)malloc(resultSize);
        memcpy(result, pos, resultSize);
        i = -1;
        break;
      }

      if (read != bufferSize || ftell(fp) >= end) {
        break;  // go onto next split
      }
    }
  }

  fclose(fp);
  free(buffer);
  return result;
}

unsigned long MP4DurationGetter::videoinfo_flip(unsigned long val) {
  unsigned long newNumber = 0;
  newNumber += (val & 0x000000FF) << 24;
  newNumber += (val & 0xFF000000) >> 24;
  newNumber += (val & 0x0000FF00) << 8;
  newNumber += (val & 0x00FF0000) >> 8;

  return newNumber;
}

unsigned long MP4DurationGetter::videoinfo_duration(char* filename) {
  unsigned long duration = 0;
  char version = 0;
  void* data = NULL;
  char* pos = NULL;
  unsigned long timescale = 0;
  unsigned long timeunits = 0;
  int bytesize = 4;

  data = videoinfo_find(filename, (void*)"mvhd", 4, 64);
  if (data == NULL) {
    goto clean;
  }

  pos = (char*)data;
  pos += 4;  // skip mvhd

  version = *pos++;
  pos += 3;  // skip flags

  if (version == 1) {
    bytesize = 8;
  } else {
    bytesize = 4;
  }

  pos += bytesize;  // skip created date
  pos += bytesize;  // skip modified date

  memcpy(&timescale, pos, 4);
  memcpy(&timeunits, pos + 4, bytesize);

  timescale = videoinfo_flip(timescale);
  timeunits = videoinfo_flip(timeunits);

  if (timescale > 0 && timeunits > 0) {
    duration = timeunits / timescale;
  }

clean:
  free(data);
  return duration;
}

QList<unsigned long> MP4DurationGetter::getBatchVideosDuration(const QStringList& filepaths) {
  QList<unsigned long> durationLst;
  durationLst.reserve(filepaths.size());
  for (const auto& path : filepaths) {
    QFileInfo fi(path);
    if (not fi.exists() or fi.suffix().toLower() != "mp4") {
      continue;
    }
    durationLst << videoinfo_duration((char*)qPrintable(path));
  }
  return durationLst;
}

QString MP4DurationGetter::DisplayVideosDuration(const QList<unsigned long>& durationLst, const QStringList& fileAbsPaths) {
  QStringList fileNames, fileDirs;
  for (const auto& pth : fileAbsPaths) {
    QFileInfo fi(pth);
    if (not fi.exists() or fi.suffix().toLower() != "mp4") {
      continue;
    }
    fileNames << fi.fileName();
    fileDirs << fi.absolutePath();
  }
  return DisplayVideosDuration(durationLst, fileNames, fileDirs);
}

QString MP4DurationGetter::DisplayVideosDuration(const QList<unsigned long>& durationLst, const QStringList& fileNames, const QStringList& fileDirs) {
  if (not(durationLst.size() == fileNames.size() and fileNames.size() == fileDirs.size())) {
    qDebug("list size must same[%d, %d, %d]", durationLst.size(), fileNames.size(), fileDirs.size());
    return "";
  }
  unsigned long totalLength = 0;
  QString dispMsg;
  for (int i = 0; i < durationLst.size(); ++i) {
    totalLength += durationLst[i];
    dispMsg += (QString::number(durationLst[i]) + '\t' + fileNames[i] + '\t' + fileDirs[i] + '\n');
  }
  return QString("Total duration:\n%1(s) of %2 video(s)\n").arg(totalLength).arg(durationLst.size()) + dispMsg;
}

QString MP4DurationGetter::DisplayVideosDuration(const QStringList& fileAbsPaths) {
  return DisplayVideosDuration(getBatchVideosDuration(fileAbsPaths), fileAbsPaths);
}
