Tux Football 0.3.1 for AmigaOS 3.x
==================================

Tux Football (Jason Wood, Christoph Brill): top-down arcade football after
Sensible Soccer and Kick Off, two teams of rendered players on a scrolling
pitch, a match in two halves with extra time and penalties, against the
computer or a friend on the same keyboard. For AmigaOS 3.x on a 68040 or
68060 with an FPU and an RTG screen, on OpenGPU's SDL 1.2 (sdl12-compat).

Base: tuxfootball_0.3.1.orig.tar.gz from Debian (sha256
44056c15572c2a3f0e3794719961915af15fef5f05596d2ef3f9e247f8a1f3e5) without
its two crowd sounds (see Data), with Debian's 0.3.1-10 patches as the
port's first commit. Branch team2/tuxfootball-amiga.

Status: plays in the lab (Instance-45 on GFOpenUp 0.8.19, 7 November
2026): the title menu after about a minute of loading (some 4,800
pictures), Return and a match against the computer: the kick-off, running
with the cursor keys, a shot, Red City's goals and the score board, the
kick-off again after a goal, half time, then Escape back to the menu and
Escape again to the Shell. Screenshots from those runs are in
amiga/screenshots (tuxfootball-title.png, tuxfootball-match.png,
tuxfootball-goal.png). The lab is muted, so the music and sound were not
heard there; the Shell shows upstream's own "FIXME" warnings as it plays.
Once, after a goal, the view stayed on an empty corner of the pitch with
the clock running until half time; upstream warns that a ball going over
the goal still counts as a goal, and the next match's goals were followed
by a proper kick-off.


What you need
-------------

- A 68040 or 68060 with an FPU, AmigaOS 3.x, an RTG card (or the
  AmigaChrome lab), and about 20 MB of free memory.
- GFOpenUp 0.8.18 or later (LIBS:OpenGPU/SDL2.module). Everything else is
  in the program.


How to play
-----------

Open the TuxFootball icon, or run TuxFootball from a Shell (TuxFootball -f
for the full screen). It opens an 800x600 window. In the menu the cursor
keys move and Return chooses. In a match the cursor keys run, Ctrl shoots
(hold it longer for a harder kick, and steer straight after it for after
touch), Shift passes; the player nearest the ball is yours. A second
player has H, J, K and L to run, G to shoot and F to pass. Escape leaves
the match, and the program from the menu. ReadMe-upstream in the drawer is
upstream's README.



Standard keys
-------------

Every AmiCradle game has the same keys on Right Amiga (GFOpenUp's
SDL2.module, after 0.8.26):

- Right Amiga + F: window or full screen, remembered for next time
- Right Amiga + P: pause (Right Amiga + P again goes on)
- Right Amiga + Q: quit
- Right Amiga + M: free the mouse pointer, or let the game take it again
- Right Amiga + S: a screenshot in the drawer's Screenshots
- Help: this game's keys (the drawer's Controls file)
- Right Amiga + F1, F2: F11, F12; Right Amiga + cursor keys: Page Up,
  Page Down, Home, End

The FULLSCREEN tooltype on the game's icon (take the brackets off) starts
it full screen.


Building
--------

    make -f amiga/Makefile.amigaos -j2 dist

with GFOpenAmigaGCC 16.2.0-open4, the OpenNDK headers, the OpenGPU kit and
the Team's static libpng, libjpeg and zlib (see the Makefile's head). The
drawer is dist/TuxFootball (and dist/TuxFootball.info).


What was changed
----------------

- src/const.h: the data is PROGDIR:data on AmigaOS, wherever the program
  is started from.
- src/gameengine.cpp: the audio is opened at 22050 Hz on AmigaOS, the rate
  the Team's mixer plays at.
- amiga/config.h stands in for configure's: HAVE_GETTEXT without
  ENABLE_NLS, so src/gettext.h's stand-ins give back the English text
  (gameengine.cpp needs its _() even without gettext).
- No SDL_image or SDL_mixer 1.2 for OpenGPU's SDL 1.2: the Team's
  amiga/sdl_image_lite.c (PNG on libpng; TGA, raw or RLE, added here for
  the two goal-post pictures) and amiga/sdl_mixer_lite.c (WAV, and the Ogg
  Vorbis music through stb_vorbis, amiga/stb_vorbis.c), from the Abe's
  Amazing Adventure and VoR ports. libmikmod 3.3.16 (amiga/libmikmod) is
  linked in with the mixer, though Tux Football has no modules.
- amiga/amiga_rwops.c (-Wl,--wrap=SDL_RWFromFile): stdio RWops, since
  sdl12-compat's could hang in the lab (Abe port).
- amiga/amiga_prefix.hpp before every C++ file: the C++ streams before the
  toolchain's pthread.h (Nikwi port).
- amiga/stack.c: 256 KB of stack.
- data/sound/crowdcheer.wav and data/sound/crowdooh.wav are the Team's own
  (see Data below); upstream's two are not in this repository or the
  drawer.


Data
----

Every data file in the drawer, and its licence:

- data/graphics/team1, team1_goalie, team2, team2_goalie (1,008 PNGs
  each): the players and goalkeepers, running, walking, standing, tackling
  and heading in eight directions, rendered by upstream from its own
  player.blend with its create.sh and rotate.py. GPL 2 or later.
- data/graphics/pitchlines (592 PNGs and tile.map): the pitch markings, and
  data/graphics/pitchtile.png, goalposttop.tga, goalpostbottom.tga: the
  grass and the goals. Upstream's, GPL 2 or later.
- data/graphics/font_white.png, font_yellow.png (SFont fonts), football.png,
  footballshadow.png, homeplayermarker.png, awayplayermarker.png, score.png,
  goal.png, foul.png, penalty.png, halftime.png, fulltime.png,
  menu_background.png, scratch.png, tuxfootball.png: upstream's, GPL 2 or
  later.
- data/sound/beep.wav, select_beep.wav, increment_beep.wav,
  decrement_beep.wav, bounce.wav, kick.wav, crowdboo.wav,
  refwhistleshort.wav, refwhistlelong.wav, refwhistletwice.wav,
  refwhistleend.wav, and the music titlemusic.ogg and ingame.ogg: they carry
  no credit or tag of their own; upstream and Debian ship them under the
  game's GPL 2 or later (Debian's copyright: "Files: *"), and they are
  shipped here on that licence, as the Team did for Vectoroids' music.
- data/sound/crowdcheer.wav, crowdooh.wav: made by the AmiCradle Team with
  amiga/tools/crowd_sounds.py (many synthesised voices through vowel
  formants, from a fixed seed), and dedicated to the public domain, CC0 1.0
  (https://creativecommons.org/publicdomain/zero/1.0/). Upstream's two
  carried a copyright tag of a free-sound-effects web site, which is not a
  free licence, so they were left out.


Licences
--------

Tux Football: GPL 2 or later (COPYING.txt); src/Font.hpp GPL 2 or later
(Lion Vollnhals, Matthias Braun); src/SFont.c GPL 2 or later or LGPL 2 or
later (Karl Bartel). Its data: see Data. stb_vorbis: public domain
or MIT (Sean Barrett). libmikmod: LGPL 2 or later (COPYING-libmikmod.txt).
The Amiga files (amiga/): the AmiCradle Team, GPL 2 or later, except the
mixer, which comes from the Team's Wizznic port and is GPL 3 or later, so
the program as built here is GPL 3 or later; the crowd sounds and the
script that makes them are CC0; the icons are our own GlowIcons
(amiga/icons/art/tuxfootball.svg).
