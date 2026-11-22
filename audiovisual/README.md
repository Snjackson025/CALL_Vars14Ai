# Call-Vars Audiovisual Upgrade (C++17 / Qt 6)

This is a new, complete **buildable project scaffold** that preserves the named Call-Vars local concepts (FOX character, FoxMathEngine, FoxMail, FoxStorage) and adds a Qt desktop shell for live Realtime audio, camera-frame input, text interaction, local math, local mail-draft logging, and basic FOX listening/speaking states.

## Important validation status

This project has **not been compiled or run in this environment**. It is an implementation baseline, not a claim of validated operation. Validate it on the target Windows 11 / Visual Studio 2022 machine before relying on it.

The FOX face in this first pass is a simple text-rendered avatar/state indicator. It is **not** a photorealistic animated fox, facial-expression recognition model, gaze tracker, or lip-sync rig. The camera is transmitted as periodic still frames, not a continuous video stream. Those are explicit next engineering stages rather than features falsely marked complete.

## Requirements

- Windows 10/11 x64
- Visual Studio 2022 with Desktop development with C++
- CMake 3.21+
- Qt 6.5+ with MSVC 2022 x64 kits: Core, Widgets, Multimedia, WebSockets, Network
- An OpenAI API key with access to the Realtime API and available quota
- Microphone, speakers/headphones, and optional webcam
- Network access to `api.openai.com`

## Build in Visual Studio 2022

1. Install a Qt 6 MSVC 2022 x64 kit and CMake.
2. Open this folder in Visual Studio (`File > Open > Folder`).
3. Configure CMake with the Qt kit. If Qt is not auto-detected, set `CMAKE_PREFIX_PATH` to the Qt MSVC kit folder, e.g. `C:/Qt/6.x.x/msvc2022_64`.
4. Build target `CallVars`.
5. Run the executable.

Command-line alternative (Developer PowerShell):

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64"
cmake --build build --config Release
```

## API key

Preferred: set `OPENAI_API_KEY` in the process environment before launching Call-Vars. Alternatively, paste it into the password field. The app does not write the key to disk. Do not commit API keys to GitHub or place them in source code. For a distributed product, replace direct desktop key use with a trusted backend that issues short-lived client credentials.

## Current implementation

- Realtime WebSocket connection (`wss://api.openai.com/v1/realtime?model=gpt-realtime`)
- Server voice activity detection and response interruption events
- Microphone PCM16 24 kHz mono streaming and PCM output playback (requires audio devices supporting this exact format)
- Periodic JPEG camera still frames added to the conversation while camera is enabled
- Text chat
- Local `/math 12 * 4` calculator
- Local `/draft ...` draft log
- Local timestamped application log under Qt's AppDataLocation
- FOX listening/speaking status display

## Known limitations / next steps

1. Add robust audio resampling for devices that do not natively support 24 kHz mono PCM.
2. Move API credentials to a trusted local/backend broker before distributing the app.
3. Add bounded audio playback buffering and a dedicated audio thread for production-grade latency.
4. Add true continuous camera/video transport or optimized frame sampling, plus user-visible camera privacy controls.
5. Add a vision analysis pipeline for expression/gaze only if explicitly designed and validated; current code merely supplies images to the model.
6. Replace the text FOX indicator with a real 2D/3D black fox rig and synchronized viseme/blendshape animation.
7. Reintegrate any additional methods from the exact November 19 source file when that source is available. This project preserves the named architecture but cannot guarantee byte-for-byte preservation of source code that was not retrievable in the current file search.

## Privacy

Camera frames are transmitted only while the app is connected and the camera is running. The API key is held in the UI/process memory only. The local application log records events and text messages; inspect `FoxCore.cpp` before using sensitive conversations, and disable or revise logging if you do not want local transcript metadata persisted.
