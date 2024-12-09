# Info

Scripts to estimate the time it takes to generate a seed by sampling
memory pages from a large entropy file.

## Quickstart

Run

```bash
make sampling; make run FNAME=entropy PAGE_SIZE=4096 SEED_PAGES=1024 ENTROPY_PAGES=2621440 REPS=10
```

This will create a file `entropy` full of zeros in the current directory.
Remove the `entropy` when you want to change either `PAGE_SIZE` or `ENTROPY_PAGES`. 

## Code style

After cloning the repository run `git config --local core.hooksPath githooks`.
This enables a script to format C source files with clangd at commit time.
Formatted files must be manually re-added to the staging area.
