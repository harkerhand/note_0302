#!/bin/bash
set -euo pipefail

# Usage: complie_exp.sh <path-to-cpp> [output-path] -- [program-args]
# If output-path is not provided, the executable will be created in the
# same directory as the .cpp file with the same basename (no .cpp).

if [ "$#" -lt 1 ]; then
	echo "Usage: $0 <path-to-cpp> [output-path] -- [program-args]"
	exit 2
fi

CPP_PATH="$1"
if [ ! -f "$CPP_PATH" ]; then
	echo "Error: file not found: $CPP_PATH"
	exit 3
fi

# Extract output path if provided
if [ "${2-}" != "" ] && [ "$2" != "--" ]; then
	OUT_PATH="$2"
	shift 2
else
	dir=$(dirname -- "$CPP_PATH")
	base=$(basename -- "$CPP_PATH")
	name="${base%.*}"
	OUT_PATH="$dir/$name.out"
	shift 1
fi

# Extract program arguments after "--"
if [ "${1-}" == "--" ]; then
	shift
	PROGRAM_ARGS=("$@")
else
	PROGRAM_ARGS=()
fi

echo "Compiling '$CPP_PATH' -> '$OUT_PATH'"

# Try to get opencv4 flags via pkg-config; if not available, compile without them
if pkg-config --exists opencv4 2>/dev/null; then
	CFLAGS=$(pkg-config --cflags --libs opencv4)
else
	CFLAGS=""
fi

g++ -std=c++23 "$CPP_PATH" -o "$OUT_PATH" $CFLAGS

echo "Running: $OUT_PATH ${PROGRAM_ARGS[*]}"
"$OUT_PATH" "${PROGRAM_ARGS[@]}"