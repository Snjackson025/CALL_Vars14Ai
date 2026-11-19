# CALL-VARS workflow

The GitHub Actions workflow for CALL-VARS lives at
`.github/workflows/call-vars.yml` in the
[snjackson044-droid/CALL_Vars14Ai](https://github.com/snjackson044-droid/CALL_Vars14Ai)
repo, on the `main` branch.

## What it does

On every push and pull request to `main` that touches `call_vars.cpp`,
`scripts/call-vars-check.sh`, or the workflow file itself, it runs one
job, **Build and smoke test**, on `ubuntu-latest`:

1. **Compile** — `g++ -std=c++17 -Wall -Wextra -o call_vars call_vars.cpp`
   with all warnings enabled. The program currently compiles with no
   warnings, so keep it that way.
2. **Smoke test 1 — start up and shut down** — runs the program with
   menu choice `0` and checks the output contains `CALL-VARS AI`,
   `FoxMath Engine: ONLINE`, `FoxMail Engine: ONLINE`, and
   `CALL-VARS shutdown complete`.
3. **Smoke test 2 — Fox Math Engine (3 x 4)** — runs the program with
   menu choice `2`, numbers `3` and `4`, then `0`, and checks the
   output contains `Standard multiplication: 12.0000` and
   `Fox multiplication: 24.0000` (FoxMath doubles the standard result).

## Running the same checks locally

```sh
sh scripts/call-vars-check.sh
```

That script mirrors the workflow exactly: same compiler flags, same
two smoke tests, same expected output. Run it before pushing.

## Adding the workflow file

The workflow file itself cannot be pushed by Muse through the GitHub
API — GitHub blocks creating `.github/workflows/*` files that way
(403 "Resource not accessible by integration"). This is the same
limitation hit with the FoxMail API workflow.

So the workflow file is added manually, on the GitHub website:

1. Open the repo at
   https://github.com/snjackson044-droid/CALL_Vars14Ai/new/main?filename=.github%2Fworkflows%2Fcall-vars.yml
2. Paste in the contents of `.github/workflows/call-vars.yml`
   (the file is also attached in chat when the workflow is delivered).
3. Make sure the filename is exactly `.github/workflows/call-vars.yml`
   — not `Callvarsyml`, not `call-vars`, and with the `.yml` ending.
4. Commit directly to `main`.

Once that commit lands, the workflow runs on the next push to `main`
that touches the paths above.

## Adding a new check

1. Add the check to `scripts/call-vars-check.sh` first, and run it
   locally until it passes.
2. Mirror the same step in `.github/workflows/call-vars.yml`.
3. Push the script change, then update the workflow file on the
   GitHub website the same way as above.
