compile_flags  = -l -g -T -Cl -Oirs -I ../knes
link_flags     = --lib-path ../knes
obj            = header.o graphics.o map.o music.o main.o sound.o
bin            = demo.nes

$(bin): $(obj)
	cl65 -m map.txt -Ln labels.txt -C nes-nrom.cfg -t none -Wl --dbgfile,$(bin).dbg $(link_flags) -o $@ $^ knes.lib

graphics.o:   graphics.c gfx/smb.h gfx/index/logo.h
music.o:      music.c song.h
sound.o:      sound.c sfx/seamen2.h
map.o:        map.c map/exported/attrib.h map/exported/map.h map/exported/metatiles.h

%.o: %.s
	cl65 -v -g -t none -l -c $<

%.o: %.c
	cl65 -t none $(compile_flags) -c $<

%.h: %.chr
	../tools/bin2head $<

%.h: %.idx
	../tools/bin2head $<

%.h: %.dmc
	../tools/bin2head $<

%.h: %.att
	../tools/bin2head $<

%.h: %.map
	../tools/bin2head $<

%.h: %.met
	../tools/bin2head $<
