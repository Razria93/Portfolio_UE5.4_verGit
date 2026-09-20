# Combat HUD source resources

Current implementation and closure (2026-09-20): [branch record](../../Docs/07_Portfolio_Documents/Portfolio_Production/34_Gameplay_Combat_HUD_Branch_Closure.md). User PIE review is confirmed. The prompt history below includes earlier concepts; the retained action icon selection is documented in [ActionSelection](Icons/ActionSelection.md). GuardBreak is a reserved asset, not a connected gameplay state.

Generated raster icons: built-in imagegen, 2026-09-18. Original pictograms, not extracted game textures. Transparent RGBA originals retained; imported UI textures capped at 256px. PIE approval does not imply readability testing on every platform and resolution.

Fonts: Oxanium (Regular/Medium), Noto Sans CJK KR (Regular); accompanying OFL licenses apply. Composite fonts use Noto as fallback. Source fonts retained for reproducible import.

Official sources: [Oxanium fonts](https://github.com/sevmeyer/oxanium/tree/master/fonts/ttf), [Oxanium license](https://github.com/sevmeyer/oxanium/blob/master/OFL.txt), [Noto Korean face](https://github.com/notofonts/noto-cjk/blob/main/Sans/OTF/Korean/NotoSansCJKkr-Regular.otf), [Noto license](https://github.com/notofonts/noto-cjk/blob/main/Sans/LICENSE).

## Prompts

### Slash
Asset type: standalone game HUD icon texture, square 1024x1024 RGBA genuinely transparent background. Subject: three clean tapered diagonal sword slash streaks rising bottom-left to top-right, middle slash longest, simple crisp ivory-white flat silhouette, consistent minimalist sci-fi combat HUD pictogram. Center in square with 22 percent empty padding on all sides. Only the three slash shapes, no circular frame, no shadow, no glow, no gradients, no text, no tile background, no checkerboard painted into image. Must remain readable at 40 pixels. Original icon, not a logo.

### sweep
Standalone square game HUD icon, one sweeping crescent sword slash, two smooth tapering curved strokes. Pure ivory-white flat silhouette on genuinely transparent RGBA background. Minimal clean smooth vector-like edges, no speckles or texture. Centered with 20 percent empty padding. Readable at 40 pixels. No circle frame, no text, no glow, no shadow, no backdrop or painted checkerboard. Original minimalist sci-fi combat pictogram.

### strike
Standalone square game HUD icon, one vertical downward sword strike with three small angular impact rays at its tip. Pure ivory-white flat silhouette on genuinely transparent RGBA background. Minimal clean smooth vector-like edges, no speckles or texture. Centered with 20 percent empty padding. Readable at 40 pixels. No circle frame, no text, no glow, no shadow, no backdrop or painted checkerboard. Original minimalist sci-fi combat pictogram.

### shield
Standalone square game HUD icon, one simple heraldic shield outline with a smaller inset shield outline. Pure ivory-white flat silhouette on genuinely transparent RGBA background. Minimal clean smooth vector-like edges, no speckles or texture. Centered with 20 percent empty padding. Readable at 40 pixels. No circle frame, no text, no glow, no shadow, no backdrop or painted checkerboard. Original minimalist sci-fi combat pictogram.

### vial
Standalone square game HUD icon, one upright small potion vial with narrow cap, simple bottle silhouette and a transparent central highlight. Pure ivory-white flat silhouette on genuinely transparent RGBA background. Minimal clean smooth vector-like edges, no speckles or texture. Centered with 20 percent empty padding. Readable at 40 pixels. No circle frame, no text, no glow, no shadow, no backdrop or painted checkerboard. Original minimalist sci-fi combat pictogram.
