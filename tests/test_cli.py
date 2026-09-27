import pathlib
import subprocess
import sys
import tempfile
import unittest

BIN = pathlib.Path(sys.argv.pop(1)).resolve()


class SearchTests(unittest.TestCase):
    def test_case_insensitive_ranked_search_and_tie_break(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / "a.txt").write_text("Rail rail cargo", encoding="utf-8")
            (root / "b.txt").write_text("rail cargo cargo", encoding="utf-8")
            (root / "c.txt").write_text("forest", encoding="utf-8")
            result = subprocess.run([str(BIN), str(root), "RAIL cargo"], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, "document,score\na.txt,3\nb.txt,3\n")
            result = subprocess.run([str(BIN), str(root), "forest"], capture_output=True, text=True)
            self.assertEqual(result.stdout, "document,score\nc.txt,1\n")

    def test_rejects_empty_query(self):
        with tempfile.TemporaryDirectory() as directory:
            (pathlib.Path(directory) / "a.txt").write_text("a", encoding="utf-8")
            result = subprocess.run([str(BIN), directory, "!?"], capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("query", result.stderr.lower())

    def test_rejects_missing_corpus(self):
        result = subprocess.run([str(BIN), "definitely-missing-corpus", "rail"], capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("directory", result.stderr.lower())


if __name__ == "__main__":
    unittest.main()
