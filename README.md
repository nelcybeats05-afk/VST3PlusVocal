# VST3+Vocals Match
New VST3 identity specifically to avoid FL Studio caching the old broken class.

What this build does:
- Load WAV / AIFF / FLAC vocal reference locally.
- Analyse RMS/dynamics, brightness/zero crossings and stereo width.
- Convert the measurements into separate MAIN, DOUBLES and ADLIBS effect settings.
- Apply the matched parameters automatically.
- No artist-name AI/prompt system in this build.
- No internet required.
- The reference analysis is heuristic: it estimates an effect character from the supplied vocal audio. It cannot recover the exact original plugin chain or settings.
