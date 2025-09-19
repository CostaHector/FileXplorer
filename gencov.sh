#!/bin/bash

# Precondition: set runnable
# chmod +x /home/ariel/code/FileXplorer/covgen.sh

# Get the absolute path of the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Build directory path (relative to script location)
BUILD_DIR="${SCRIPT_DIR}/build/FileXplorerTest_Desktop_Qt_5_15_2_GCC_64bit-Debug"

# Coverage report output directory
COVERAGE_REPORT_DIR="${BUILD_DIR}/coverage_report"

# Default action: generate coverage and open report
ACTION="generate_and_open"

# Parse command line options
while getopts "qn" opt; do
  case $opt in
    q) ACTION="quiet_generate" ;;
    n) ACTION="open_only" ;;
    \?)
      echo "Usage: $0 [-q] [-n]"
      echo "Options:"
      echo "  -q : Quiet mode (generate report without opening)"
      echo "  -n : Open existing report only (no generation)"
      exit 1
      ;;
  esac
done

# Log function
log() {
  if [ "$ACTION" != "quiet_generate" ]; then
    echo "$1"
  fi
}

# Error function
error() {
  echo "ERROR: $1" >&2
  exit 1
}

# Check if build directory exists
if [ ! -d "${BUILD_DIR}" ]; then
  error "Build directory not found - ${BUILD_DIR}. Please ensure the project has been built."
fi

# Change to build directory
cd "${BUILD_DIR}" || error "Failed to enter build directory"

# Open existing report only
if [ "$ACTION" = "open_only" ]; then
  REPORT_INDEX="${COVERAGE_REPORT_DIR}/index.html"
  if [ -f "${REPORT_INDEX}" ]; then
    log "Opening existing coverage report..."
    xdg-open "${REPORT_INDEX}" > /dev/null 2>&1 &
    exit 0
  else
    error "Coverage report not found. Please generate the report first."
  fi
fi

# Clean coverage data
if [ "$ACTION" = "force_rebuild" ]; then
  log "Cleaning previous coverage data..."
  # do following when remove some file, clean and compile again
  # find . -name "*.gcda" -delete
  # find . -name "*.gcno" -delete
  # lcov --zerocounters --directory . > /dev/null

  log "Running tests to generate coverage data..."
  ./FileXplorerTest || error "Test execution failed"
fi

# Generate coverage data
log "Generating coverage data..."
/usr/bin/lcov --capture --directory . --output-file coverage.info \
    --exclude "/home/ariel/Qt/*" \
    --exclude "/usr/include/*" \
    --exclude "/usr/local/include/*" \
    --exclude "*/TestCase/*" \
    --exclude "*/unittest/*" \
    --exclude "*/build/*"

# 关键修复：检查覆盖率文件是否有效
if [ ! -s coverage.info ]; then
  error "Coverage file is empty or missing. Check for errors above."
fi

# Generate HTML report
log "Generating HTML report..."
genhtml coverage.info --output-directory "${COVERAGE_REPORT_DIR}" || error "Failed to generate HTML report"

# Check if report was generated successfully
REPORT_INDEX="${COVERAGE_REPORT_DIR}/index.html"
if [ ! -f "${REPORT_INDEX}" ]; then
  error "Failed to generate coverage report index"
fi

# Open report unless in quiet mode
if [ "$ACTION" != "quiet_generate" ]; then
  log "Opening coverage report in browser..."
  xdg-open "${REPORT_INDEX}" > /dev/null 2>&1 &
fi

log "Coverage report generated: ${REPORT_INDEX}"
log "Script execution completed successfully"
