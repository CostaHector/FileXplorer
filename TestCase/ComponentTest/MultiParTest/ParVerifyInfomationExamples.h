#ifndef PARVERIFYINFOMATIONEXAMPLES_H
#define PARVERIFYINFOMATIONEXAMPLES_H

#include "ParVerifyInfomation.h"

namespace ParVerifyInfomationExamples {
constexpr const char CLI_OUTPUT_ALL_FILES_COMPLETE[]{R"(Parchive 2.0 client version 1.3.3.5 by Yutaka Sawada

Base Directory	: "C:\Users\aria\AppData\Local\Temp\FileXplorerTestOnly-xVIKDr\"
Recovery File	: "C:\Users\aria\AppData\Local\Temp\FileXplorerTestOnly-xVIKDr\MediaFileNeedBackup.mp4.par2"
CPU thread	: 6 / 12
CPU cache limit	: 64 KB, 1024 KB (32)
CPU extra	: AVX2 CLMUL
Memory usage	: Auto (8958 MB available), Fast SSD

PAR File list :
         Size :  Filename
          412 : "MediaFileNeedBackup.mp4.par2"
          528 : "MediaFileNeedBackup.mp4.vol0+1.par2"

PAR File total size	: 940
PAR File possible count	: 2

Recovery Set ID		: 6F0DF5A7C68CA03F1EA9FD592B1B2EC1
Input File Slice size	: 48
Input File total count	: 1
Recovery Set file count : 1
Creator : par2j v1.3.3

Input File list      :
         Size  Slice :  Filename
           46      1 : "MediaFileNeedBackup.mp4"

Input File total size	: 46
Input File Slice count	: 1

Loading PAR File       :
 Packet Slice Status   :  Filename
      4     0 Good     : "MediaFileNeedBackup.mp4.par2"
      5     1 Good     : "MediaFileNeedBackup.mp4.vol0+1.par2"

Recovery Slice count	: 1
Recovery Slice found	: 1

Verifying Input File   :
         Size Status   :  Filename
            = Complete : "MediaFileNeedBackup.mp4"

Complete file count	: 1
Misnamed file count	: 0
Damaged file count	: 0
Missing file count	: 0

Input File Slice found	: 1

Input File Slice avail	: 1
Input File Slice lost	: 0

All Files Complete
)"};


constexpr const char CLI_OUPUT_READY_TO_RENAME[]{R"(Parchive 2.0 client version 1.3.3.5 by Yutaka Sawada

Base Directory	: "C:\Users\Ariel\Documents\TestQuickBar\"
Recovery File	: "C:\Users\Ariel\Documents\TestQuickBar\Another file need backup 2.txt.par2"
CPU thread	: 6 / 6
CPU cache limit	: 64 KB, 768 KB (36)
CPU extra	: AVX2 CLMUL
Memory usage	: Auto (1422 MB available), Fast SSD

PAR File list :
         Size :  Filename
          520 : "Another file need backup 2.txt.par2"
         1164 : "Another file need backup 2.txt.vol0+1.par2"
         1164 : "Another file need backup 2.txt.vol1+1.par2"

PAR File total size     : 2848
PAR File possible count : 3

Recovery Set ID         : 164BC982ED2C1F1F915F4E4370F8FF7F
Input File Slice size   : 576
Input File total count  : 1
Recovery Set file count : 1
Creator : par2j v1.3.3

Input File list      :
         Size  Slice :  Filename
         3451      6 : "Another file need backup 2.txt"

Input File total size   : 3451
Input File Slice count  : 6

Loading PAR File       :
 Packet Slice Status   :  Filename
      4     0 Good     : "Another file need backup 2.txt.par2"
      5     1 Good     : "Another file need backup 2.txt.vol0+1.par2"
      5     1 Good     : "Another file need backup 2.txt.vol1+1.par2"

Recovery Slice count    : 2
Recovery Slice found    : 2

Verifying Input File   :
         Size Status   :  Filename
            - Missing  : "Another file need backup 2.txt"

Complete file count	: 0

Searching misnamed file: 1
         Size Status   :  Filename
         3451 Found    : "Another file need backup 55.txt"
            = Misnamed : "Another file need backup 2.txt"

Misnamed file count	: 1
Damaged file count	: 0
Missing file count	: 0

Input File Slice found  : 6

Input File Slice avail  : 6
Input File Slice lost   : 0

Ready to rename 1 file(s)
)"};

constexpr const char CLI_OUTPUT_READY_TO_REPAIR[]{R"(Parchive 2.0 client version 1.3.3.5 by Yutaka Sawada

Base Directory	: "C:\Users\Ariel\Documents\TestQuickBar\"
Recovery File	: "C:\Users\Ariel\Documents\TestQuickBar\Another file need backup 2.txt.par2"
CPU thread	: 6 / 6
CPU cache limit	: 64 KB, 768 KB (36)
CPU extra	: AVX2 CLMUL
Memory usage	: Auto (1686 MB available), Fast SSD

PAR File list :
         Size :  Filename
          520 : "Another file need backup 2.txt.par2"
         1164 : "Another file need backup 2.txt.vol0+1.par2"
         1164 : "Another file need backup 2.txt.vol1+1.par2"

PAR File total size     : 2848
PAR File possible count : 3

Recovery Set ID         : 164BC982ED2C1F1F915F4E4370F8FF7F
Input File Slice size   : 576
Input File total count  : 1
Recovery Set file count : 1
Creator : par2j v1.3.3

Input File list      :
         Size  Slice :  Filename
         3451      6 : "Another file need backup 2.txt"

Input File total size   : 3451
Input File Slice count  : 6

Loading PAR File       :
 Packet Slice Status   :  Filename
      4     0 Good     : "Another file need backup 2.txt.par2"
      5     1 Good     : "Another file need backup 2.txt.vol0+1.par2"
      5     1 Good     : "Another file need backup 2.txt.vol1+1.par2"

Recovery Slice count    : 2
Recovery Slice found    : 2

Verifying Input File   :
         Size Status   :  Filename
         3397        0 : "Another file need backup 2.txt"

Complete file count	: 0

Searching misnamed file: 1
         Size Status   :  Filename

Misnamed file count	: 0
Damaged file count	: 1
Missing file count	: 0

Input File Slice found  : 0

Finding available slice:
         Size Status   :  Filename
         3397 Damaged  : "Another file need backup 2.txt"
            =        5 : "Another file need backup 2.txt"

Input File Slice found  : 5

Comparing lost slice    : 1 within 5
Null byte slice count   : 0
Reversible slice count  : 0
Duplicate slice count   : 0

Counting available slice:
 Avail /  Slice :  Filename
     5 /      6 : "Another file need backup 2.txt"

Input File Slice avail  : 5
Input File Slice lost   : 1

Ready to repair 1 file(s)
)"};

constexpr const char CLI_OUPUT_NEED_MORE_SLICES_TO_REPAIR[]{R"(Parchive 2.0 client version 1.3.3.5 by Yutaka Sawada

Base Directory	: "C:\Users\Ariel\Documents\TestQuickBar\"
Recovery File	: "C:\Users\Ariel\Documents\TestQuickBar\Another file need backup 2.txt.par2"
CPU thread	: 6 / 6
CPU cache limit	: 64 KB, 768 KB (36)
CPU extra	: AVX2 CLMUL
Memory usage	: Auto (1517 MB available), Fast SSD

PAR File list :
         Size :  Filename
        19404 : "Another file need backup 2.txt.par2"
      9941452 : "Another file need backup 2.txt.vol0+94.par2"

PAR File total size     : 9960856
PAR File possible count : 2

Recovery Set ID         : 3D57DB646AFE034107AD0FAE73DE83C3
Input File Slice size   : 104252
Input File total count  : 1
Recovery Set file count : 1
Creator : par2j v1.3.3

Input File list      :
         Size  Slice :  Filename
     98830848    948 : "Another file need backup 2.txt"

Input File total size   : 98830848
Input File Slice count  : 948

Loading PAR File       :
 Packet Slice Status   :  Filename
      4     0 Good     : "Another file need backup 2.txt.par2"
    116    94 Good     : "Another file need backup 2.txt.vol0+94.par2"

Recovery Slice count    : 94
Recovery Slice found    : 94

Verifying Input File   :
         Size Status   :  Filename
            - Missing  : "Another file need backup 2.txt"

Complete file count	: 0

Searching misnamed file: 1
         Size Status   :  Filename

Misnamed file count	: 0
Damaged file count	: 0
Missing file count	: 1

Input File Slice found  : 0

Finding available slice:
         Size Status   :  Filename

Input File Slice found  : 0

Comparing lost slice    : 948 within 0
Null byte slice count   : 0
Reversible slice count  : 0
Duplicate slice count   : 0

Counting available slice:
 Avail /  Slice :  Filename
     0 /    948 : "Another file need backup 2.txt"

Input File Slice avail  : 0
Input File Slice lost   : 948

Need 854 more slice(s) to repair 1 file(s)
)"};

constexpr const char CLI_OUPUT_WHEN_REPAIR[] {R"(Parchive 2.0 client version 1.3.3.5 by Yutaka Sawada

Base Directory	: "CLI_OUPUT_WHEN_REPAIR_PATH\"
Recovery File	: "CLI_OUPUT_WHEN_REPAIR_PATH\MusicFileNeedBackup.mp4.par2"
CPU thread	: 6 / 12
CPU cache limit : 64 KB, 1024 KB (32)
CPU extra	: AVX2 CLMUL
Memory usage	: Auto (9389 MB available), Fast SSD

PAR File list :
         Size :  Filename
          472 : "MusicFileNeedBackup.mp4.par2"
          888 : "MusicFileNeedBackup.mp4.vol0+1.par2"

PAR File total size	: 1360
PAR File possible count	: 2

Recovery Set ID		: 24FBEE2FDFB6E2E78DA6EBD30625B6FF
Input File Slice size	: 348
Input File total count	: 1
Recovery Set file count : 1
Creator : par2j v1.3.3

Input File list      :
         Size  Slice :  Filename
         1053      4 : "MusicFileNeedBackup.mp4"

Input File total size	: 1053
Input File Slice count	: 4

Loading PAR File       :
 Packet Slice Status   :  Filename
      4     0 Good     : "MusicFileNeedBackup.mp4.par2"
      5     1 Good     : "MusicFileNeedBackup.mp4.vol0+1.par2"

Recovery Slice count	: 1
Recovery Slice found	: 1

Verifying Input File   :
         Size Status   :  Filename
         1050        0 : "MusicFileNeedBackup.mp4"

Complete file count	: 0

Searching misnamed file: 1
         Size Status   :  Filename

Misnamed file count	: 0
Damaged file count	: 1
Missing file count	: 0

Input File Slice found	: 0

Finding available slice:
         Size Status   :  Filename
         1050 Damaged  : "MusicFileNeedBackup.mp4"
            =        3 : "MusicFileNeedBackup.mp4"

Input File Slice found	: 3

Comparing lost slice	: 1 within 3
Null byte slice count	: 0
Reversible slice count	: 0
Duplicate slice count	: 0

Counting available slice:
 Avail /  Slice :  Filename
     3 /      4 : "MusicFileNeedBackup.mp4"

Input File Slice avail	: 3
Input File Slice lost	: 1

Ready to repair 1 file(s)

Repairing file  :
100.0%
100.0%

Verifying repair: 1
 Status   :  Filename
 Repaired : "MusicFileNeedBackup.mp4"

Repaired file count	: 1
Input File Slice avail	: 4

Repaired successfully
)"
};

constexpr const char CLI_OUPUT_WHEN_RENAME[] {R"(Parchive 2.0 client version 1.3.3.5 by Yutaka Sawada

Base Directory	: "CLI_OUPUT_WHEN_RENAME_PATH\"
Recovery File	: "CLI_OUPUT_WHEN_RENAME_PATH\comicFile.txt.par2"
CPU thread	: 6 / 12
CPU cache limit : 64 KB, 1024 KB (32)
CPU extra	: AVX2 CLMUL
Memory usage	: Auto (9192 MB available), Fast SSD

PAR File list :
         Size :  Filename
          404 : "comicFile.txt.par2"
       384472 : "comicFile.txt.vol0+1.PAR2"

PAR File total size	: 384876
PAR File possible count	: 2

Recovery Set ID		: AD0BF6F2B170724CEAF294D35CF89DC5
Input File Slice size	: 384000
Input File total count	: 1
Recovery Set file count : 1
Creator : QuickPar 0.9

Input File list      :
         Size  Slice :  Filename
         1566      1 : "comicFile.txt"

Input File total size	: 1566
Input File Slice count	: 1

Loading PAR File       :
 Packet Slice Status   :  Filename
      4     0 Good     : "comicFile.txt.par2"
1
      5     1 Good     : "comicFile.txt.vol0+1.PAR2"

Recovery Slice count	: 1
Recovery Slice found	: 1

Verifying Input File   :
         Size Status   :  Filename
            - Missing  : "comicFile.txt"

Complete file count	: 0

Searching misnamed file: 1
         Size Status   :  Filename
1
         1566 Found    : "comicFile111.txt"
            = Misnamed : "comicFile.txt"

Misnamed file count	: 1
Damaged file count	: 0
Missing file count	: 0

Input File Slice found	: 1

Input File Slice avail	: 1
Input File Slice lost	: 0

Ready to rename 1 file(s)

Correcting file : 1
 Status   :  Filename
 Restored : "comicFile.txt"

Restored file count	: 1

Repaired successfully
)"};

inline const ParVerifyInfomation& GetAllFilesCompleteStruct() {
  static const ParVerifyInfomation info{CLI_OUTPUT_ALL_FILES_COMPLETE};
  return info;
}

inline const ParVerifyInfomation& GetReadyToRenameStruct() {
  static const ParVerifyInfomation info{CLI_OUPUT_READY_TO_RENAME};
  return info;
}

inline const ParVerifyInfomation& GetReadyToRepairStruct() {
  static const ParVerifyInfomation info{CLI_OUTPUT_READY_TO_REPAIR};
  return info;
}

inline const ParVerifyInfomation& GetNeedMoreSlicesToRepairStruct() {
  static const ParVerifyInfomation info{CLI_OUPUT_NEED_MORE_SLICES_TO_REPAIR};
  return info;
}

inline const ParVerifyInfomation& GetVerifyUntrustableStruct() {
  static const ParVerifyInfomation info;
  return info;
}

} // namespace ParVerifyInfomationExamples

#endif // PARVERIFYINFOMATIONEXAMPLES_H
