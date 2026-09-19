# Selected combat action icons (2026-09-19)

The user-selected `icon` folder supplied the initial selection. GuardBreak was
subsequently regenerated to match Guard's scale while retaining the broad tear.
Source directory: `C:/Users/starb/.codex/generated_images/01a0aa52-3514-7ad2-a428-3f55ab522edb/icon`.

| Source PNG | Project source / texture suffix | HUD position |
| --- | --- | --- |
| exec-6f7b9cf5-7d5f-4821-bddf-cefc58210616.png | ActionRush | Small top circle |
| exec-2b50c9a8-b67b-4bec-9e19-0b574b10697c.png | ActionGuard | Top |
| exec-166acf45-2072-401d-b819-eca089a682d3.png | ActionDodge | Left |
| exec-830d3b25-a05d-4c7f-afa5-01cdf8794d0e.png | ActionCounter | Right; arrow only |
| exec-e9571ad4-b19d-433d-ab28-98626aa7110b.png | ActionExecution | Bottom; selected sword/impact symbol |
| exec-8a4c4910-03fb-49a4-b5ff-04f344a1db56.png (generated_images parent folder) | ActionGuardBreak | Reserved alternate, not displayed |

The selected Vial matches the existing potion source and remains unchanged.
Textures live at `/Game/08_UI/CombatHUD/T_HUDAction*` and can be recreated with
`-run=CCombatHUDAssets -CreateMissing` (never overwrites existing assets).
Settings: UI group, EditorIcon compression, no mipmaps, maximum size 256.

This is presentation integration only. Pending tint and TODO remain intentionally:
no availability, parry/perfect-dodge success, guard-break transition or cooldown
is inferred from an icon. GuardBreak is loaded as a reserved style asset only.
Only the six selected Action PNGs and Vial.png are retained here. Unused BE
placeholder sources, alternative drafts, previews and superseded revision notes
were removed during cleanup. The asset creation commandlet uses the same set.

## GuardBreak revision

ImageGen used ActionGuard as the base and the original
exec-2b83517b-adb8-4921-b6f5-d21deeb7acac.png as the tear reference.
The broad zigzag is visually reproduced, not a pixel-exact copied mask.
Alpha >=128 bounds on the 1254x1254 sources: Guard 704x804, GuardBreak 702x816;
both centers are (626.5,635.5).

Prompt: Edit the guard base without moving or resizing it. Apply the reference's
single broad zigzag tear, with two large directional reversals, asymmetric width,
an open top notch and a tapered bottom. Preserve the exterior silhouette and
padding; flat white icon on transparent background, no extra elements.
