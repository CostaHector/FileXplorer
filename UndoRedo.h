#ifndef UNDOREDO_H
#define UNDOREDO_H
#include <QPair>
#include "FileOperation/FileOperation.h"
class OperationStream{
public:
    FileOperation::BATCH_COMMAND_LIST_TYPE doCmd;
    FileOperation::BATCH_COMMAND_LIST_TYPE recoverCmd;
};

#include<QStack>
#include<QPair>

class UndoRedo {
public:
    using UNDO_REDO_RETURN = QPair<bool, OperationStream>;

    inline auto Do(const FileOperation::BATCH_COMMAND_LIST_TYPE& cmd) -> bool{
        FileOperation::BATCH_COMMAND_LIST_TYPE srcCommand;
        const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(cmd, srcCommand);
        const auto isAllSucceed = exeRetEle.first;
        const auto& recover_cmd = exeRetEle.second;
        undoList.append(OperationStream{cmd, recover_cmd});
        return isAllSucceed;
    }

    inline auto Undo() -> UNDO_REDO_RETURN{
        if (not undoAvailable()){
            qDebug("Skip Cannot undo");
            return qMakePair<bool, OperationStream>(true, OperationStream());
        }
        OperationStream ele = undoList.pop();
        const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(ele.recoverCmd, ele.doCmd);
        const auto isAllSucceed = exeRetEle.first;
        redoList.append(ele); // you can not just update do_cmd in ele. Because it is not equivelant as rmfile has no recover
        // compromise method: when pass the do_cmd list into it
        return qMakePair<bool, OperationStream>(isAllSucceed, ele);
    }
    inline auto Redo() -> UNDO_REDO_RETURN{
        if (not redoAvailable()){
            qDebug("Skip Cannot redo");
            return qMakePair<bool, OperationStream>(true, OperationStream());
        }
        OperationStream ele = redoList.pop();
        const FileOperation::EXECUTE_RETURN_TYPE& exeRetEle = FileOperation::executer(ele.doCmd, ele.recoverCmd);
        const auto isAllSucceed = exeRetEle.first;
        undoList.append(ele);
        return qMakePair<bool, OperationStream>(isAllSucceed, ele);
    }

    inline auto undoAvailable() -> bool{
        return not undoList.isEmpty();
    }

    inline auto redoAvailable() -> bool{
        return not redoList.isEmpty();
    }

    inline auto clear() -> void{
        decltype(undoList) emptyUndoList;
        undoList.swap(emptyUndoList);
        decltype(undoList) emptyRedoList;
        redoList.swap(emptyRedoList);
    }

    QStack<OperationStream> undoList;
    QStack<OperationStream> redoList;
};

extern UndoRedo g_undoRedo;
#endif // UNDOREDO_H
