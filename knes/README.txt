KNES library for CC65
---------------------
by thefox // aspekt 2011 :: thefox@aspekt.fi :: http://thefox.aspekt.fi

This is a replacement NES library for CC65. I have removed bunch of "useless"
stuff from the default nes.lib and replaced crt0.s with a new version.

It probably won't work on the current "official" CC65 release, so use the
snapshot version from ftp://ftp.musoftware.de/pub/uz/cc65/snapshot/

Check knes/knes.h for info about what's included. Also there's an incomplete
platformer sample game under the "demo" directory.  It demonstrates scrolling,
collision checking, audio, DPCM, sprite 0 hit etc.

In "tools" directory there's a utility to convert binary files to C headers.

I'm open to any suggestions. If you have some, my e-mail address is up there.
You can also reach me from http://nesdev.parodius.com/bbs/

Some tips for programming the NES in C:
  - Compile with the "-Cl" switch, this makes local parameters static. Otherwise
    they're allocated from the CC65 software emulated stack, which is understandably
    very slow. One drawback to this is that every single temporary variable will now
    take a byte in RAM, even if it's used very rarely. It might be a good idea to have
    some generic temporary global variables for that purpose instead.
  - Use #pragma bss-name and/or data-name to place variables on the zero page.
  - Avoid passing parameters to functions -- use global variables. If you need
    parameters, use the __fastcall__ calling convention, which can pass two byte
    variables in the A and X registers.
  - Use the optimizer ("-Oirs" switch) BUT be aware that it might in some cases
    produce broken code. One such case is when you read the controllers by strobing
    $4016, then reading it eight times. The first read is optimized away. Of course
    when you're using this library you can simply use read_joy().

Version history
---------------

0.1.1
  Fixed to work with latest development version of CC65.. and some other changes
  which I can't remember because the project isn't under version control.. duh!
0.1
  Initial release
