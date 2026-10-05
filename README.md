# VST3+Vocals AI FINAL

Stable identity kept from the working build:
- Product: VST3+Vocals
- Bundle ID: com.29yuro.vst3plusvocals
- Plugin code: Vpvl

Features:
- Main Vocal / Doubles / Adlibs parameter sections
- Real audio DSP for gain, filtering, compression-style dynamics, saturation, air/EQ coloration, reverb, delay
- Parallel generated doubles and adlib-style effect layers
- Local text chain assistant, including an "Absent" style preset
- WAV/AIFF/FLAC/MP3 reference loading
- Local reference-song analysis that measures level, crest/dynamics, brightness and stereo width and maps them to Main/Doubles/Adlibs effect settings
- State saving and host automation
- Windows Server 2022 / VS2022 GitHub build
- CI verifies binary and moduleinfo.json before uploading artifact

Important: reference matching is heuristic analysis of the complete audio file. It does not magically isolate vocals from a mastered song, and the text assistant is local rule-based chain generation rather than a cloud LLM.
