#!/bin/sh
# Verify disk usage: run du from a path and print in GB for comparison with nnn.
# Usage: ./verify-du.sh [path]
# Default path: current directory (.)
# Compare the output with nnn's du display when run from the same path.

path="${1:-.}"
echo "Path: $path"
echo ""

# Count files (like nnn does)
# nnn counts all non-directory entries: regular files, symlinks, devices, pipes, sockets, etc.
# nnn also counts files it cannot stat (permission denied) as non-directories
# -P: do not follow symbolic links
# -xdev: do not cross filesystem boundaries
# ! -type d: count everything except directories
# Do NOT suppress errors - count entries we can't access too
if command -v find >/dev/null 2>&1; then
	echo "File count (all non-directory entries):"
	# First, try to count including permission errors
	# find will still list the path even if it can't stat it
	file_count=$(find -P "$path" -xdev ! -type d 2>&1 | grep -v "Permission denied" | wc -l)

	# Alternative: count accessible files + count permission denied entries
	accessible=$(find -P "$path" -xdev ! -type d 2>/dev/null | wc -l)
	denied=$(find -P "$path" -xdev 2>&1 >/dev/null | grep -c "Permission denied" || echo 0)

	echo "  Accessible: $accessible files"
	echo "  Permission denied: $denied entries"
	echo "  Estimated total: $((accessible + denied)) files"

	# Debug: show breakdown by type
	echo ""
	echo "  Breakdown of accessible files:"
	echo "    Regular files: $(find -P "$path" -xdev -type f 2>/dev/null | wc -l)"
	echo "    Symlinks: $(find -P "$path" -xdev -type l 2>/dev/null | wc -l)"
	echo "    Other (devices, pipes, sockets): $(find -P "$path" -xdev \( -type c -o -type b -o -type p -o -type s \) 2>/dev/null | wc -l)"
	echo ""

	# Count directories
	echo "Directory count:"
	dir_count=$(find -P "$path" -xdev -type d 2>/dev/null | wc -l)
	echo "  $dir_count directories"
	echo ""
fi

# GNU du: -s = summary, default block size 1024 bytes
# So "du -s" output * 1024 = bytes
if command -v du >/dev/null 2>&1; then
	echo "du -s (1024-byte blocks, same as 'du -sh'):"
	blocks=$(du -s "$path" 2>/dev/null | cut -f1)
	if [ -n "$blocks" ]; then
		# bytes = blocks * 1024; GB = bytes / 1024^3
		bytes=$((blocks * 1024))
		gb=$(echo "scale=3; $bytes / 1073741824" | bc 2>/dev/null)
		echo "  $blocks 1K-blocks"
		echo "  ${gb} GB (bytes: $bytes)"
	else
		echo "  (failed or permission denied)"
	fi
	echo ""

	# Also show in 512-byte blocks (st_blocks units, what nnn uses for disk usage)
	echo "Equivalent in 512-byte blocks (nnn st_blocks units):"
	if [ -n "$blocks" ]; then
		blocks512=$((blocks * 2))
		echo "  $blocks512 512-byte blocks"
	fi
fi
