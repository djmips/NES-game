These are LUA scripts for Mappy for generating NES compatible data.

LUA scripts can be run by drag&dropping in to Mappy

For these to work, the palette has to have 16 entries, and must be organized like
it is in the NES, so that background is mirrored correctly etc.

Note: currently metatile #0 shouldnt be used in Mappy, as it isn't exported!!

So, in practice when exporting a map one has to:
- export the map data (metatile indices): "Export NES map.lua"
- export map attributes: "Export NES attribute lookup.lua"
- convert 16x16->8x8 (using Mappy)
  (in "Useful functions")
- convert from 4-bit to 2-bit "Convert 4bit to 2bit (NES).lua"
- optimize graphics [but not blocks] (using Mappy)
  - MapTools -> Remove Unused/Duplicate
- export metatile table: "Export NES tile lookup.lua"
- export 8x8 tile data: "Export NES charset.lua"


How to import image file to Mappy:
- File -> New Map
  - 16x16 tiles, width and height based on image size (div by 16), paletted (8bit)
- palette organizing is a PITA
  - if copying from a game, one way is to copy from Nintendulator palette viewer
    and organize by hand in Photoshop, however problems arise when several colorsets
    use the same colors
- MapTools -> Useful functions -> Create map from big picture (8 bit BMP)
  - optimize

