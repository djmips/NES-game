-- Export NES 16x16 metatile indices after map has been converted 16x16->8x8
-- thefox//aspekt 2009

function num_to_char ( number )
 return ( string.char ( math.mod ( math.mod ( number, 256 ) + 256, 256 ) ) )
end

function writeByte ( file, number )
 file:write ( num_to_char( number )) -- x>>0
end

function main ()
 if mappy.msgBox ("NES 16x16 table", "This will export NES 16x16 metatile table of 8x8 converted map\n\nRun the script (you will be prompted for a filename to save as)?", mappy.MMB_OKCANCEL, mappy.MMB_ICONQUESTION) == mappy.MMB_OK then

  local w = mappy.getValue(mappy.MAPWIDTH)
  local h = mappy.getValue(mappy.MAPHEIGHT)

  if (w == 0) then
   mappy.msgBox ("NES 16x16 table", "You need to load or create a map first", mappy.MMB_OK, mappy.MMB_ICONINFO)
  else
 
   local isok,asname = mappy.fileRequester (".", "NES metatiles (*.met)", "*.met", mappy.MMB_SAVE)
   if isok == mappy.MMB_OK then

    if (not (string.sub (string.lower (asname), -4) == ".met")) then
     asname = asname .. ".met"
    end

    outas = io.open (asname, "wb")
    local blk = 0
    local maxblk = mappy.getValue (mappy.NUMBLOCKSTR)
    -- TODO: check that maxblk % 4 == 0 (has to be after 16x16->8x8 conversion)
    
    local shit = 0
    while shit < 4 do
     local crap = 0
     -- skip the first 4 tiles, as mappy insists on having a blank metatile as index #0
     blk = 4 + shit
     -- write 256 bytes for each quadrant of a metatile, first write 256bytes of tile #0, then tile #1, etc, total 1024 bytes
     while crap < 256 do
      local blkval
      if blk < maxblk then
       blkval = mappy.getBlockValue (blk, mappy.BLKBG) - 1
      else
       blkval = 0
      end
      writeByte (outas, blkval)
      blk = blk + 4
      crap = crap + 1
     end
     shit = shit + 1
    end
    outas:close ()

   end
  end
 end
end

test, errormsg = pcall( main )
if not test then
    mappy.msgBox("Error ...", errormsg, mappy.MMB_OK, mappy.MMB_ICONEXCLAMATION)
end
