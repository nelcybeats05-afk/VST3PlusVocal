# VST3 VocalChain+ Design V3

This version keeps the loading-safe pass-through processor architecture that successfully opened in FL Studio.

Changes in this build are deliberately UI/editor-side:
- Dark purple/blue VocalChain+ layout based on the approved mockup.
- Clearly labeled Main Vocal controls: Tune, Comp, De-Esser, EQ, Saturation, Air, Reverb, Delay, Level.
- Doubles and Adlibs labeled separately.
- More Effects area explicitly names EQ, Compressor, Reverb, Delay, Saturation and Chorus.
- Reference audio loader accepts vocal WAV as the preferred source, plus normal WAV/AIFF/FLAC.
- Analyse changes the visible controls based on basic reference measurements.
- AI Assistant has been removed.

Important: to preserve the FL-loading baseline, this design build does not yet put the effect DSP into the processor. The next safe step is to add DSP incrementally after this exact build is confirmed `ok` in FL Studio.
