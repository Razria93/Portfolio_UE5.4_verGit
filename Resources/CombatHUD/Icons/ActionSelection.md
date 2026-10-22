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

## Lethal execution (2026-09-27)

`ActionExecutionLethal.png` is a new white angular skull on transparent RGBA,
generated with built-in imagegen. Standard execution retains the existing sword
icon. The native HUD defaults bind the new texture
`/Game/08_UI/CombatHUD/T_HUDActionExecutionLethal`; the creation commandlet imports
it with the same UI settings above. No pulse, enlargement or lethal-only tint.

Prompt: Create one game combat HUD icon texture: LETHAL EXECUTION represented by
a single angular front-facing skull. Flat pure white solid silhouette, two large
transparent angular eye sockets, tiny triangular transparent nose, simplified
squared jaw with just two tooth gaps. Clean minimalist futuristic combat
pictogram, readable at 32 pixels. Centered square composition, skull occupies
65 percent of canvas width and 72 percent of height, even clear padding.
Genuinely transparent RGBA background. No circle, no badge, no text, no sword,
no crossbones, no glow, no shadows, no gradients, no texture, no black filled
background. This will be tinted white or cyan by game UI. Output one transparent
PNG asset.

Validation (2026-10-22, G4 based on main `1e02c8a5`): default Unity
PortfolioEditor Win64 Development build and all 93 Portfolio automation tests
passed, including all 15 CombatHUD tests. Existing warning-only tests: 2.
Actual rendered PIE verified Standard/Lethal readiness and active icons,
interruption/death cleanup, and the separate low-health hint. UI-inclusive
screenshots show the existing sword and new skull at their actual HUD size.
The texture loads with UI compression, UI LOD group and no mipmaps. Existing
assets were not resaved. See `../ActionStateIntegration.md` for fixture limits.

Boundary revalidation (2026-10-22): cumulative C1/C2/C3/C4 snapshots passed
default Unity builds, all 90/91/93/93 Portfolio tests, and the dodge/montage
asset audits. The new texture and source PNG enter together in C2, where the
ExecutionIcons test first runs. Final rendered PIE repeated all six scenarios
with 99 behavior and 34 capture assertions passing (17 UI-inclusive images).
After temporary instrumentation removal, the default Unity build and all 93
tests passed again. No production code or asset changed during this validation;
only these result documents were updated after execution.
