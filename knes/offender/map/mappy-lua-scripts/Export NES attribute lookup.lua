-- Export attribute bytes for all 256 metatiles
-- Go thru all metatiles (blocks), go thru all pixels in them and find out which attribs they use
-- thefox//aspekt 2009

function num_to_char ( number )
 return ( string.char ( math.mod ( math.mod ( number, 256 ) + 256, 256 ) ) )
end

function writeByte ( file, number )
 file:write ( num_to_char( number )) -- x>>0
end

function main ()
 if mappy.msgBox ("Export attributes (NES)", "This exports attribute bytes for all metatiles.\n\nRun the script?", mappy.MMB_OKCANCEL, mappy.MMB_ICONQUESTION) == mappy.MMB_OK then

  local w = mappy.getValue(mappy.BLOCKWIDTH)
  local h = mappy.getValue(mappy.BLOCKHEIGHT)
  local wp = mappy.andVal(w,-4)

  if (mappy.getValue(mappy.MAPWIDTH) == 0) then
   mappy.msgBox ("Export attributes (NES)", "You need to load or create a map first", mappy.MMB_OK, mappy.MMB_ICONINFO)
  else
  
   -- TODO: check that BLOCKDEPTH == 8

   local isok,asname = mappy.fileRequester (".", "Attribute files (*.att)", "*.att", mappy.MMB_SAVE)
   if isok == mappy.MMB_OK then
   
    if (not (string.sub (string.lower (asname), -4) == ".att")) then
     asname = asname .. ".att"
    end
    
    outas = io.open (asname, "wb")

    -- local isok,tstrt,tend = mappy.doDialogue ("Export Tiles as BMPs", "Range:", "0,"..(mappy.getValue(mappy.NUMBLOCKGFX)-1), mappy.MMB_DIALOGUE2)
    -- if true then
    
    local isok,adjust = mappy.doDialogue ("Export binary file", "Multiply exported values by (4 for SRMB):", "1", mappy.MMB_DIALOGUE1)
    if isok == mappy.MMB_OK then

    local tstrt = 1 -- skip first (blank) tile
    local tend = mappy.getValue(mappy.NUMBLOCKGFX)
    local tnum = tstrt
    while (tnum < tend) do
     local y = 0
     local attrval = 0
     while y < h do
      local x = 0
      while x < w do
-- getPixel returns an index for 8bit, or a,r,g,b for other depths
       local i,r,g,b = mappy.getPixel (x, y, tnum)
       if (mappy.getValue(mappy.BLOCKDEPTH) == 8) then 
        i = math.mod(math.floor(i / 4), 4) -- i is the 2-bit attribute value for this pixel
        if i ~= 0 then
         -- TODO: if attrval ~= 0 -> error
         -- NOTE: "Peli" requires that this value is shifted left 2 bits o_X
         -- attrval = 4 * i
         attrval = adjust * i
        end
       end
       x = x + 1
      end
      y = y + 1
     end
     tnum = tnum + 1
     
     writeByte (outas, attrval)

    end
    
    -- pad the file to 256 bytes
    while tnum <= 256 do
     writeByte (outas, 0)
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
