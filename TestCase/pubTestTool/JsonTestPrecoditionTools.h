#ifndef JSONTESTPRECODITIONTOOLS_H
#define JSONTESTPRECODITIONTOOLS_H
#include "SceneInfoManager.h"

namespace JsonTestPrecoditionTools {
constexpr char JSON_CONTENTS[] {R"({
    "Bitrate": "",
    "Cast": [
        "Cast1NotExist",
        "Cast2NotExist"
    ],
    "Detail": "This is just a json example.",
    "Duration": 0,
    "Hot": [
    ],
    "Name": "My Good Boy",
    "Rate": 4,
    "Resolution": "720p",
    "Size": "126113854",
    "Studio": "StudioNotExist",
    "Tags": [
        "nonporn"
    ],
    "Uploaded": "20231022"
})"
};

// "Studio" should be GameTurbo
// "Cast" should be GGG YYYYY
// fileName should be renamed to ${Name}.json
// tags should set to tag1
constexpr char JSON_CONTENTS_A_RANK_IN_MODEL[] {R"({
    "Bitrate": "",
    "Cast": [
        "Empty Cast A 1",
        "Empty Cast A 2"
    ],
    "Detail": "This is just a json example.",
    "Duration": 36000,
    "Hot": [
    ],
    "Name": "GameTurbo - A rank - GGG YYYYY",
    "Rate": 4,
    "Resolution": "720p",
    "Size": "10240",
    "Studio": "Empty Studio A",
    "Tags": [
        "Empty Tag A"
    ],
    "Uploaded": "20231022"
})"
};

// "Studio" should be GameTurbo
// "Cast" should be "XX YY", "ZZ DD EE"
// fileName should be renamed to ${Name}.json
// tags should set to tag1,tag2,tag3
constexpr char JSON_CONTENTS_B_RANK_IN_MODEL[] {R"({
    "Bitrate": "",
    "Cast": [
        "Empty Cast B 1",
        "Empty Cast B 2"
    ],
    "Detail": "This is just b json example.",
    "Duration": 3600,
    "Hot": [
    ],
    "Name": "GameTurbo - B rank - XX YY and ZZ DD EE",
    "Rate": 4,
    "Resolution": "720p",
    "Size": "10240",
    "Studio": "Empty Studio B",
    "Tags": [
        "Empty Tag B"
    ],
    "Uploaded": "20231022"
})"
};
}
#endif  // JSONTESTPRECODITIONTOOLS_H
