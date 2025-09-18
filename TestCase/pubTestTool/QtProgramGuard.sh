#!/bin/bash

PID_FILE="/tmp/qt_program_guard.pid"

# 检查第三个参数
if [ "$3" = "start" ]; then
    # 将当前进程 PID 写入文件
    echo $$ > ${PID_FILE}
    sleep $1
    pkill -f "$2"
else
    # 新逻辑：杀死 PID 文件中的进程
    if [ -f ${PID_FILE} ]; then
        pid=$(cat ${PID_FILE})
        # 验证 PID 是否有效
        if [ -n "$pid" ] && [ "$pid" -gt 0 ] 2>/dev/null; then
            kill $pid
            echo "Killed process with PID: $pid"
        else
            echo "Invalid PID in file: $pid"
        fi
    else
        echo "PID file not found: ${PID_FILE}"
    fi
fi
