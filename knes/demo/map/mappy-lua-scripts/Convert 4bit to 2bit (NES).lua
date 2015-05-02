-- Convert 4bit to 2bit (NES)
-- thefox//aspekt 2009

function main ()
 if mappy.msgBox ("Convert 4bit to 2bit (NES)", "This converts all tiles from 4-bit to 2-bit graphics, removing attribute bits for NES.\n\nRun the script?", mappy.MMB_OKCANCEL, mappy.MMB_ICONQUESTION) == mappy.MMB_OK then

  local w = mappy.getValue(mappy.BLOCKWIDTH)
  local h = mappy.getValue(mappy.BLOCKHEIGHT)
  local wp = mappy.andVal(w,-4)

  if (mappy.getValue(mappy.MAPWIDTH) == 0) then
   mappy.msgBox ("Convert 4bit to 2bit (NES)", "You need to load or create a map first", mappy.MMB_OK, mappy.MMB_ICONINFO)
  else
  
   -- TODO: check that BLOCKDEPTH == 8

   -- local isok,asname = mappy.fileRequester (".", "BMP files (*.bmp)", "*.bmp", mappy.MMB_SAVE)
   if true then

    -- local isok,tstrt,tend = mappy.doDialogue ("Export Tiles as BMPs", "Range:", "0,"..(mappy.getValue(mappy.NUMBLOCKGFX)-1), mappy.MMB_DIALOGUE2)
    if true then

    local tstrt = 0
    local tend = mappy.getValue(mappy.NUMBLOCKGFX)
    local tnum = tstrt
    while (tnum < tend) do
     local y = 0
     while y < h do
      local x = 0
      while x < w do
-- getPixel returns an index for 8bit, or a,r,g,b for other depths
       local i,r,g,b = mappy.getPixel (x, y, tnum)
       if (mappy.getValue(mappy.BLOCKDEPTH) == 8) then 
        i = math.mod(i, 4)
        mappy.setPixel (x, y, tnum, i);
       end
       x = x + 1
      end
      y = y + 1
     end
     tnum = tnum + 1

    end
    mappy.updateScreen()
    end
   end
  end
 end
end

test, errormsg = pcall( main )
if not test then
    mappy.msgBox("Error ...", errormsg, mappy.MMB_OK, mappy.MMB_ICONEXCLAMATION)
end
