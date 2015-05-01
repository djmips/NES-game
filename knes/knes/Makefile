compile_flags  = -g -T -Cl -Oirs

obj = crt0.o knes.o _read_joy.o
bin = knes.lib
unwanted_crap = _scrsize.o cclear.o chline.o clock.o clrscr.o color.o \
	cputc.o crt0.o cvline.o get_tv.o gotox.o gotoxy.o gotoy.o mainargs.o \
	ppu.o ppubuf.o randomize.o revers.o setcursor.o sysuname.o waitvblank.o \
	wherex.o wherey.o \

$(bin): $(obj)
	cp original\nes.lib knes.lib
	ar65 d $@ $(unwanted_crap)
	ar65 a $@ $^
    
%.o: %.s
	cl65 -v -g -t none -c $<
	
%.o: %.c
	cl65 -t none $(compile_flags) -c $<
