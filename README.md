# detelev_visu



The coordinate axes have been moved back to the true world origin and now render in correct (unscaled, untranslated) space alongside the properly scaled zones:

What changed:

Removed the axes from overlayTransform (they were previously scaled/translated with the model center and artificial scale factor).
Added a world-space axes node directly under root with length 1500 mm (covering car and zones).
Left all zone transforms untouched (still 1000x scaling only) so their positions remain correct.
Build succeeded; runtime log shows all 18 valid zones created as before.
Zones unaffected:

Zone centers printed are exactly the same pattern as earlier (still in mm after internal 1000x scale).
No rotation/translation logic for zones was altered—only the axes node relocation.
Next optional refinements (if you want):

Adjust axis length (current: 1500 mm). For a longer baseline: change to 2500.
Add a label (X/Y/Z) at axis ends for clarity.
Add a toggle flag (e.g., --no-axes) if you want to hide them for screenshots.
Let me know which (if any) you’d like and I can implement the next step. Otherwise you’re back to a clean, correct coordinate reference.
