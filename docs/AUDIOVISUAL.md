# CALL-VARS Audiovisual Upgrade

A Qt 6 desktop version of CALL-VARS that sits alongside the original console
program (`call_vars.cpp`) in this repo. It adds a window with live voice
through OpenAI's Realtime API, camera still frames, text chat, and the local
FoxMath / FoxMail kept in.

## Files

All of it lives in `audiovisual/`:

- `main.cpp`, `CallVarsWindow.h` / `CallVarsWindow.cpp` — the Qt window,
  Realtime WebSocket, microphone/speaker streaming, camera frames.
- `FoxCore.h` / `FoxCore.cpp` — FoxCharacter, FoxMathEngine, FoxMail,
  FoxStorage (local log + drafts).
- `CMakeLists.txt` — CMake project, C++17, Qt 6.5+ (Core, Widgets,
  Multimedia, WebSockets, Network).
- `README.md` — full build instructions and limits.

The original `call_vars.cpp` is untouched.

## Build on Windows

Needs Visual Studio 2022, CMake 3.21+, and a Qt 6.8 MSVC 2022 x64 kit
(Core, Widgets, Multimedia, WebSockets, Network):

```powershell
cmake -S audiovisual -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/msvc2019_64"
cmake --build build --config Release
```

Run `build/Release/CallVars.exe` with `OPENAI_API_KEY` set, or paste the key
into the app. The key is kept in memory only and never written to disk.

## CI

`.github/workflows/call-vars-qt.yml` builds it on `windows-latest` (Qt 6.8.3
installed by `jurplel/install-qt-action`), on pushes touching `audiovisual/`
and every day at 06:00 UTC. It checks the build succeeds and
`build/Release/CallVars.exe` exists. It does not run the app — a GUI program
cannot run headless on the runner.

Qt version note: Qt 6.5.3 was tried first and failed — its
`qvarlengtharray.h` uses Microsoft's `stdext::checked_array_iterator`, which
the current MSVC compiler removed, so the build failed with 16 `stdext`
errors (run #1, 2026-11-22). Qt 6.8.3 no longer uses it.

## Known limits (2026-11-22)

- Never compiled or run yet — the first GitHub run proves the build.
- Voice needs an OpenAI key with Realtime API access and available credits.
- The FOX face is a text indicator for now, not the animated portrait.
- `/math` gives the standard result only; `FoxMultiply` exists in the code
  but is not wired into the `/math` command yet.
- Two missing includes were added so it can compile: `<QTextCursor>` in
  `CallVarsWindow.cpp` and `<QStringList>` in `FoxCore.cpp`.
