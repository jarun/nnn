#!/usr/bin/env python3
"""Run with python3 misc/test/test-listing-selection.py (requires a C compiler and ncursesw)."""

import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest


SOURCE = Path(os.environ.get("NNN_TEST_SOURCE", Path(__file__).resolve().parents[2] / "src/nnn.c"))
HARNESS = r'''
#define main nnn_main
#include NNN_TEST_SOURCE
#undef main

int main(int argc, char **argv)
{
	struct stat sb;
	size_t i;
	int selected;

	if (argc != 6 || fstat(STDIN_FILENO, &sb) || sb.st_size <= 0 || sb.st_size > UINT_MAX)
		return 2;
	selbufpos = selbuflen = sb.st_size;
	pselbuf = malloc(selbuflen);
	if (!pselbuf || fread(pselbuf, 1, selbuflen, stdin) != selbuflen || pselbuf[selbufpos - 1])
		return 2;
	for (i = 0; i < selbufpos; ++i)
		nselected += !pselbuf[i];
	selected = nselected;
	listpath = *argv[1] ? argv[1] : NULL;
	listroot = argv[2];
	selpath = argv[3];
	g_state.selmode = 1;
	xstrsncpy(g_tmpfpath, "/tmp/", TMP_LEN_MAX);
	tmpfplen = 6;
	for (i = 0; i < strtoul(argv[5], NULL, 10); ++i)
		endselection(atoi(argv[4]));
	if (nselected != selected || g_state.selmode != !atoi(argv[4]) || selbufpos > selbuflen)
		return 3;
	if (fwrite(pselbuf, 1, selbufpos, stdout) != selbufpos)
		return 4;
	free(pselbuf);
	return 0;
}
'''


class ListingSelectionTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        cls.root = Path(cls.temp.name)
        harness = cls.root / "selection.c"
        harness.write_text(HARNESS)
        cls.binary = cls.root / "selection"
        curses = subprocess.check_output(["pkg-config", "--cflags", "--libs", "ncursesw"], text=True)
        command = shlex.split(os.environ.get("CC", "cc"))
        command += ["-std=c11", "-Wall", "-Wextra", "-Wshadow", "-Werror", "-DNORL", "-O1", "-g"]
        command += shlex.split(os.environ.get("NNN_TEST_CFLAGS", ""))
        command += ['-DNNN_TEST_SOURCE="' + str(SOURCE.resolve()) + '"',
                    str(harness), "-o", str(cls.binary)]
        command += shlex.split(curses) + ["-lpthread"]
        subprocess.run(command, check=True)

    def check_selection(self, paths, expected, root="/real/root", listing="/tmp/nnn-listing",
                        endmode=True, repeats=1):
        with tempfile.TemporaryDirectory(dir=self.root) as directory:
            work = Path(directory)
            selection = work / "selection"
            data = lambda entries: b"\0".join(os.fsencode(p) for p in entries) + b"\0"
            source = work / "input"
            source.write_bytes(data(paths))
            with source.open("rb") as stream:
                result = subprocess.run(
                    [str(self.binary), listing, root, str(selection), str(int(endmode)), str(repeats)],
                    stdin=stream, cwd=work, capture_output=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertFalse((work / "marker").exists(), "a filename was executed as shell source")
            self.assertEqual(result.stderr, b"")
            self.assertEqual(result.stdout, data(expected))
            if listing:
                self.assertEqual(selection.read_bytes(), data(expected)[:-1])
            else:
                self.assertFalse(selection.exists())

    def test_literal_directory_names(self):
        for name in ("plain", "with spaces", "quote'", "double\"quote", "amp&ersand",
                     "pipe|bar", "back\\slash", "line\nbreak", "trailing\n", "tab\there",
                     "中文", "x'$(touch marker)'", "x'`touch marker`'", "$(touch marker)"):
            with self.subTest(name=name):
                root = "/real/" + name
                self.check_selection(["/tmp/nnn-listing/file"], [root + "/file"], root=root)

    def test_multiple_and_nested_paths(self):
        self.check_selection(["/tmp/nnn-listing/a", "/tmp/nnn-listing/sub/b", "/elsewhere/c"],
                             ["/real/root/a", "/real/root/sub/b", "/elsewhere/c"])

    def test_newline_followed_by_slash_is_not_a_separator(self):
        self.check_selection(["/tmp/nnn-listing/line\n/child", "/tmp/nnn-listing/end\n"],
                             ["/real/root/line\n/child", "/real/root/end\n"])

    def test_expansion_beyond_original_allocation(self):
        root = "/real/" + "long" * 60
        paths = ["/tmp/nnn-listing/" + str(i) for i in range(200)]
        self.check_selection(paths, [root + "/" + str(i) for i in range(200)], root=root)

    def test_repeated_normalization(self):
        self.check_selection(["/tmp/nnn-listing/a", "/elsewhere/b"],
                             ["/real/root/a", "/elsewhere/b"], repeats=3)

    def test_sibling_prefix_is_unchanged(self):
        self.check_selection(["/tmp/nnn-listing-other/a"], ["/tmp/nnn-listing-other/a"])

    def test_listing_directory_itself(self):
        self.check_selection(["/tmp/nnn-listing"], ["/real/root"])

    def test_root_prefix_preserves_existing_serialization(self):
        self.check_selection(["/tmp/nnn-listing/a"], ["//a"], root="/")

    def test_selection_mode_is_preserved_when_requested(self):
        self.check_selection(["/tmp/nnn-listing/a"], ["/real/root/a"], endmode=False)

    def test_non_listing_selection_is_unchanged(self):
        self.check_selection(["/ordinary/a"], ["/ordinary/a"], listing="")


if __name__ == "__main__":
    unittest.main()
