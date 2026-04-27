#ifndef RENAMERKEY_H
#define RENAMERKEY_H

#include "KV.h"

namespace RenamerKey {
using namespace GeneralDataType;
using namespace RawVariant;
using namespace ValueChecker;
constexpr KV NAME_EXT_INDEPENDENT{"RenamerKey/NAME_EXT_INDEPENDENT", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV INCLUDING_DIR{"RenamerKey/INCLUDING_DIR", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV INS_OLD_STR_LIST{"RenamerKey/INS_OLD_STR_LIST", //
                              Var{"\n"                       // insert nothing
                                  "BB\n"
                                  "- 360p\n"
                                  "- 480p\n"
                                  "- 516p\n"
                                  "- 720p\n"
                                  "- 1080p\n"
                                  "- 4K\n"
                                  "- FHD\n"
                                  // insert new items before me
                                  "- UHD"},                          //
                              GeneralDataType::Type::MULTI_LINE_STR, //
                              GeneralCStrChecker};
constexpr KV REP_OLD_STR_LIST{"RenamerKey/REP_OLD_STR_LIST",    //
                              Var{"\n"                          // replace nothing
                                  "\\s\\s+"                     // used for remove extra continous space
                                  "([a-z])([A-Z])\n"            // aA. used for add space between lowercase and uppercase
                                  "(\\d{4})(\\d{2})(\\d{2})]\n" // 19700101->1970-01-01. used for date add hypen
                                  "BB\n"
                                  "- 360p\n"
                                  "- 480p\n"
                                  "- 516p\n"
                                  "- 720p\n"
                                  "- 1080p\n"
                                  // insert new items before me
                                  "- 2160p"},                        //
                              GeneralDataType::Type::MULTI_LINE_STR, //
                              GeneralCStrChecker};
constexpr KV NEW_STR_LIST{"RenamerKey/NEW_STR_LIST", //
                          Var{"\n"                   // replaced with empty(i.e. delete)
                              " \n"                  // replaced with single space
                              "\\1 \\2\n"            // used for add space
                              "\\1 - \\2 - \\3\n"    // used for date
                              "HD\n"
                              "FHD\n"
                              // insert new items before me
                              "UHD"},
                          GeneralDataType::Type::MULTI_LINE_STR, //
                          GeneralCStrChecker};
constexpr KV INSERT_INDEXES_LIST{"RenamerKey/INSERT_INDEXES_LIST", //
                                 Var{"0\n"                         // insert at index 0
                                     "260\n"                       // insert after back
                                     // insert new items before me
                                     "100"},                            //
                                 GeneralDataType::Type::MULTI_LINE_STR, //
                                 GeneralCStrChecker};
constexpr KV ARRANGE_SECTION_INDEX{"RenamerKey/ARRANGE_SECTION_INDEX", Var{"1,2"}, GeneralDataType::Type::PLAIN_STR, GeneralCStrChecker};
constexpr KV REGEX_ENABLED{"RenamerKey/REGEX_ENABLED", Var{false}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};

constexpr KV NUMERIAZER_UNIQUE_EXT_COUNTER{"RenamerKey/NUMERIAZER_UNIQUE_EXT_COUNTER", Var{true}, GeneralDataType::Type::PLAIN_BOOL, GeneralBoolChecker};
constexpr KV NUMERIAZER_NO_FORMAT{"RenamerKey/NUMERIAZER_NO_FORMAT", //
                                  Var{" %1\n"                        //
                                      " - %1\n"                      //
                                      // insert new items before me
                                      " (%1)"},                          //
                                  GeneralDataType::Type::MULTI_LINE_STR, //
                                  GeneralBoolChecker};
constexpr KV NUMERIAZER_NO_FORMAT_DEFAULT_INDEX{"RenamerKey/NUMERIAZER_NO_FORMAT_DEFAULT_INDEX", Var{0}, GeneralDataType::Type::PLAIN_INT, GeneralIntChecker};
} // namespace RenamerKey

#endif // RENAMERKEY_H
