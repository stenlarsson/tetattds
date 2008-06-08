          _______________________________________________
         /\_   _\  ___\_   _\  __ \_   _\_   _\    \   __\
         \//\  \/\  ___\\  \/\     \\  \//\  \/\ \  \__   \
           \ \__\ \_____\\__\ \__\__\\__\\ \__\ \____\_____\
            \/__/\/_____//__/\/__/__//__/ \/__/\/____/_____/

                       TetattDS v3.0 by Flatware
                  http://blog.dev-scene.com/flatware

About
-----
Tetattds is a game for Nintendo DS based on the game Tetris Attack by
Nintendo / Intelligent Systems. The aim is to line up three or more
panels in a stack to make them 'pop'. The game is over when the stack
reaches the top of the screen. If more than three blocks are removed
it is called a 'combo'. If pieces falling down lines up it is called
a 'chain'.


Endless
-------
Survive as long as possible and try to set a new highscore. Extra
points are awarded for making chains and combos.


Vs Self
-------
Chains and combos generate garbage which is dropped from above. This
mode is intended as a practice for Wifi play.


Wifi play
---------
Play up to 4 opponents locally or over Internet. Chains and combos
generate garbage which is dropped on one of your opponents and the
last survivor is the winner.

To play over Internet you need to have a wireless accesspoint supported
by dswifi. You also need to have entered wifi settings in a game like
Mario Kart DS.

You can connect to our official server att wallbraker.ath.cx, but
you could also run your own server on a PC. You might need
to open up port 13687 (UDP) in your firewall.

There are two flavours of the server. The GUI version requires
.NET Framework 2.0, which can be downloaded from Microsoft and
only works in Windows. The other, text-based version, can be
compiled in Linux, Cygwin or msys. You need to download the
source package for this, and then type "make" in the server
directory.

To play locally you don't need an accesspoint. One player hosts
and the other players join.


Known problems
--------------
* The different levels needs to be balanced.


Changelog
---------

1.1 (20060831)
  * Implemented a new udp based network protocol
    This fixes the crashing issues and is also faster
  * Default server address is now ted.getmyip.com instead of nothing
  * Fixed chat-parsing, correctly finds where to break up lines
  * Added menu background
  * New colors in the menus
  * Prevent sending empty chat messages
  * Typing notification
  * Made Start function as Enter in the text dialogs
  
1.2 (20061006)
  * Fixed some crashes in wifi mode
  * Players return to menu after dying in 3+ player games so they can
    chat and quit.
  * A new server gui written in C++/CLI (needs .NET Framework 2.0)
  * Close button added to keyboard dialog and others

2.0 (20070218)
  * New graphics!
  * New sound and music!
  * Now saves highscores to FAT if available (otherwise it uses SRAM)
  * Server now works on ppc
  * Bug fixes
  * Improved building from source
  * Built with latest devkitpro

3.0 (20080608)
  * Local Wifi using liblobby
  * Host game on DS
  * SDL port for PC
  * Lots of changes in the code

Credits
-------
Programing:
Thomas Ericsson
Sten Larsson
Jakob Bornecrantz
Gustav Munkby

Graphics:
Sten Larsson
Jakob Bornecrantz

Sound and Music:
Fredrik Hultin


Acknowledgments
---------------
This game uses the following libraries:

libnds    (http://www.devkitpro.org/)
dswifi    (http://www.akkit.org/dswifi/)
libmikmod (http://mikmod.raphnet.net/)
liblobby  (http://forum.gbadev.org/viewtopic.php?p=147290#147290)

Contact
-------
Thomas Ericsson    ted.ericsson@gmail.com
Sten Larsson       sten.larsson@gmail.com
Jakob Bornecrantz  wallbraker2@hotmail.com
Fredrik Hultin     noname@izik.se