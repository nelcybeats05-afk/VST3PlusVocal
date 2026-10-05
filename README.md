# VST3+Vocals SAFE
This build intentionally returns to the same minimal processor architecture as the version that loaded successfully in FL Studio.

It adds only editor-side WAV reference analysis. The audio processor remains a scanner-safe pass-through in this diagnostic build. The WAV analyser estimates separate Main / Doubles / Adlibs settings and displays them in the plugin UI.

Why: the previous expanded DSP builds were reported by FL Studio as error. This isolates the scanner/loading problem before DSP is reintroduced.

New identity:
- Product: VST3+Vocals SAFE
- Bundle: com.29yuro.vst3plusvocalssafe
- Plugin code: Vpvs

The GitHub artifact now contains the OUTER `VST3+Vocals SAFE.vst3` folder, ready to copy directly into `C:\Program Files\Common Files\VST3`.
