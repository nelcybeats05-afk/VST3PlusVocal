# VST3+Vocals LoadFirst

Diagnostic/stable build based on the minimal processor architecture.

Current stage:
- VST3 audio processor is pass-through only.
- No effect parameters or DSP are created during plugin scan.
- No AI assistant.
- Reference loader accepts WAV, AIFF and FLAC.
- Isolated vocal WAV is recommended; a normal song can also be loaded.
- Analyse currently verifies decoding and extracts basic level/peak/brightness measurements.
- UI keeps the Main Vocal / Doubles / Adlibs layout direction.

This stage deliberately prioritizes getting an `ok` result in FL Studio before effect DSP is reintroduced.
