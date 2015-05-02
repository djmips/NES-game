-- Export NES 2bpp charset
-- thefox//aspekt 2009

function num_to_char ( number )
 return ( string.char ( math.mod ( math.mod ( number, 256 ) + 256, 256 ) ) )
end

function writeByte ( file, number )
 file:write ( num_to_char( number )) -- x>>0
end

function main ()
 if mappy.msgBox ("Export NES 2bpp charset", "This exports NES 2bpp charset file.\n\nRun the script?", mappy.MMB_OKCANCEL, mappy.MMB_ICONQUESTION) == mappy.MMB_OK then

  local w = mappy.getValue(mappy.BLOCKWIDTH)
  local h = mappy.getValue(mappy.BLOCKHEIGHT)
  local wp = mappy.andVal(w,-4)

  if (mappy.getValue(mappy.MAPWIDTH) == 0) then
   mappy.msgBox ("Export NES 2bpp charset", "You need to load or create a map first", mappy.MMB_OK, mappy.MMB_ICONINFO)
  else
  
   -- TODO: check that BLOCKDEPTH == 8 and w = h = 8

   local isok,asname = mappy.fileRequester (".", "NES charset files (*.chr)", "*.chr", mappy.MMB_SAVE)
   if isok == mappy.MMB_OK then
   
    if (not (string.sub (string.lower (asname), -4) == ".chr")) then
     asname = asname .. ".chr"
    end

    -- local isok,tstrt,tend = mappy.doDialogue ("Export Tiles as BMPs", "Range:", "0,"..(mappy.getValue(mappy.NUMBLOCKGFX)-1), mappy.MMB_DIALOGUE2)
    if true then
    
    outas = io.open (asname, "wb")

    local tstrt = 1 -- skip the first tile...
    local tend = mappy.getValue(mappy.NUMBLOCKGFX)
    -- TODO: check NUMBLOCKGFX <= 256 (note: the first graphic, tho, is unused)
    local tnum = tstrt
    while (tnum < tend) do
     -- first 8 bytes (bit #0)
     local y = 0
     while y < h do
      local x = 0
      local byt = 0
      while x < w do
-- getPixel returns an index for 8bit, or a,r,g,b for other depths
       local i,r,g,b = mappy.getPixel (x, y, tnum)
       if (mappy.getValue(mappy.BLOCKDEPTH) == 8) then 
        i = math.mod(i, 2)
        byt = (2 * byt) + i
       end
       x = x + 1
      end
      writeByte (outas, byt)
      y = y + 1
     end
     -- 2nd 8 bytes (bit #0)
     y = 0
     while y < h do
      local x = 0
      local byt = 0
      while x < w do
-- getPixel returns an index for 8bit, or a,r,g,b for other depths
       local i,r,g,b = mappy.getPixel (x, y, tnum)
       if (mappy.getValue(mappy.BLOCKDEPTH) == 8) then 
        i = math.mod(math.floor(i / 2), 2)
        byt = (2 * byt) + i
       end
       x = x + 1
      end
      writeByte (outas, byt)
      y = y + 1
     end

     
     tnum = tnum + 1

    end
    
    -- pad the file to 4k (if over 4k no padding)
    while tnum <= 256 do
     local c = 0
     while c < 16 do
      writeByte (outas, 0)
      c = c + 1
     end
     tnum = tnum + 1
    end
    
    outas:close ()
    
    end
   end
  end
 end
end

test, errormsg = pcall( main )
if not test then
    mappy.msgBox("Error ...", errormsg, mappy.MMB_OK, mappy.MMB_ICONEXCLAMATION)
end
