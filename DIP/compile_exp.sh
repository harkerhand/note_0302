#!/bin/bash
set -euo pipefail

# Usage: complie_exp.sh <path-to-cpp> [output-path]
# If output-path is not provided, the executable will be created in the
# same directory as the .cpp file with the same basename (no .cpp).

if [ "$#" -lt 1 ]; then
	echo "Usage: $0 <path-to-cpp> [output-path]"
	exit 2
fi

CPP_PATH="$1"
if [ ! -f "$CPP_PATH" ]; then
	echo "Error: file not found: $CPP_PATH"
	exit 3
fi

if [ "${2-}" != "" ]; then
	OUT_PATH="$2"
else
	dir=$(dirname -- "$CPP_PATH")
	base=$(basename -- "$CPP_PATH")
	name="${base%.*}"
	OUT_PATH="$dir/$name.out"
fi

echo "Compiling '$CPP_PATH' -> '$OUT_PATH'"

# Try to get opencv4 flags via pkg-config; if not available, compile without them
if pkg-config --exists opencv4 2>/dev/null; then
	CFLAGS=$(pkg-config --cflags --libs opencv4)
else
	CFLAGS=""
fi

g++ "$CPP_PATH" -o "$OUT_PATH" $CFLAGS

echo "Running: $OUT_PATH"
"$OUT_PATH"