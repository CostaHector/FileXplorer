#include "FileOperation.h"
const QMap<QString, std::function<FileOperation::RETURN_TYPE(const QStringList&)>>
    FileOperation::LambdaTable={{"rmfile",rmfileAgent}, {"rmpath",rmpathAgent},
                                  {"rmdir",rmdirAgent}, {"moveToTrash",moveToTrashAgent},
                                  {"touch",touchAgent}, {"mkpath",mkpathAgent},
                                  {"rename",renameAgent},
                                  {"cpfile",cpfileAgent}, {"cpdir",cpdirAgent}};

