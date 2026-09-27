# Inverted index search

Search a small local text corpus with a C++17 inverted index. It counts each query term in each document, ranks documents by total term frequency, and breaks score ties by filename.

```sh
c++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o search
./search fixtures/corpus "rail cargo"
python3 tests/test_cli.py ./search
```

The demo prints [examples/results.csv](examples/results.csv). The corpus is one directory containing `.txt` files; subdirectories and symlinks are ignored. Search indexes ASCII letters and digits case-insensitively, treats punctuation as separators, and counts a repeated query term once. A document matching any query term appears in the CSV result. It does not do stemming, phrase search, or Unicode normalization.

The corpus may contain up to 10,000 documents of at most 10 MB each. File names containing CSV punctuation are rejected. A missing/empty corpus or query exits nonzero with a diagnostic. The program writes results to standard output and never changes source files.
