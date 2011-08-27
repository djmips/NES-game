rem obj = crt0.o knes.o _read_joy.o

copy original\nes.lib knes.lib

ar65 d knes.lib _scrsize.o cclear.o chline.o clock.o clrscr.o color.o cputc.o crt0.o cvline.o get_tv.o gotox.o gotoxy.o gotoy.o mainargs.o ppu.o ppubuf.o randomize.o revers.o setcursor.o sysuname.o waitvblank.o wherex.o wherey.o

ar65 a knes.lib crt0.o knes.o _read_joy.o
