# Caviar format evidence

Read this before changing the CVR loader or renderer layouts. External research
is a seed; original instructions, actual asset boundaries and source bindings
must corroborate a change. This does not qualify a new match or library exclusion.

The user supplied the [Caviar wiki](https://alphacentauri2.info/wiki/Caviar.html)
(revision 7011, consulted 2026-10-31). Its newer specification credits
PlotinusRedux; the older Ford Prefect section explicitly warns of errors.
The linked [CVR-Colorizer](https://github.com/uaEquals42/CVR-Colorizer) is a
separate GPL-2.0 implementation to consult with attribution, not an original
renderer oracle or a replacement dependency.

## Correlations that guide recovery

- The documented nested tag/length format agrees with `CvrChunkHeader` and
  the loader's eight-byte header consumption. Length includes the header.
- The geometry container is `0x04040200`. Its `0x04040201` payload is read by
  the misleadingly named `vox_read_colour_chunks` (`0x00638CC0`). Preserve
  the additionally supported `0x04040202` path; omission from the wiki is
  not evidence that the original lacked it.
- The animation container is `0x04040300`. Its three child tags load one-byte
  flags, twelve-byte positions and thirty-six-byte matrices per frame through
  `vox_read_geometry_chunks` (`0x00638DE0`). This reader and `CvrPart` need
  semantic naming review: `colours_` is an inherited name, not proof of color
  storage. Its renderer consumer also uses bit 7 to skip an object.
- `step_count_` is loaded from the frame-counter chunk. Do not describe it as
  inherently a rotation count merely because callers reduce a frame index
  modulo it. Keep signed arithmetic where the original uses signed division.
- The packed `CvrShape` prefix and stream boundary agree with existing
  `+0x26` count / `+0x2A` payload evidence. The five-byte zero tails observed
  after the checked group walks remain unexplained.

## Conflicts and limits

Palette shading starts at `0x302`: two leading bytes plus 256 RGB triples.
The wiki's `0x102` offset is inconsistent with that layout. The existing
pre-version-7 expansion path remains necessary; modern samples cannot replace it.
Animation positions have a twelve-byte stride, not the printed `0x1C`.
The printed `0/4/8` offsets for three byte-sized voxel fields must not be
copied into a padded structure; compact items require separate verification.

The proposed file-axis order and normal decoder are investigation leads,
not established internal coordinate conventions or a replacement for the
pinned integer tables. Preserve unknown flags, extra geometry/shadow chunks,
malformed-input behavior and existing claims. Four bounded sample walks,
including ACOLPOD matching the published example, do not prove every asset
version or complete rendering behavior. Recompute detailed observations from
assets into ignored caches; keep only essential verified layout evidence in source.
