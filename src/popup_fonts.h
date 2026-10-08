#pragma once

class Font;

// Application popup/interlude font policy, published by config_popups.
// These borrowed pointers do not belong to the generic Font subsystem.
// Borrowed pointers into FontTable, published by config_popups; no Font
// lifetime belongs to these caches.
extern Font *PopupMainFont;
extern Font *PopupMainBoldFont;
extern Font *PopupMainItalicFont;
extern Font *PopupSmallFont;
extern Font *PopupSmallBoldFont;
extern Font *PopupSmallItalicFont;
extern Font *PopupMenuFont;
extern Font *PopupMenuBoldFont;
extern Font *PopupMenuItalicFont;
extern Font *PopupMediumFont;
extern Font *PopupMediumBoldFont;
extern Font *PopupMediumItalicFont;
extern Font *InterludeNormalFont;
extern Font *InterludeSecondFont;
extern Font *InterludeThirdFont;
extern Font *PopupLargeFont;
extern Font *PopupLargeBoldFont;
extern Font *PopupLargeItalicFont;
extern Font *PopupMapLabelFont;
extern Font *PopupDefaultWindowFont;
