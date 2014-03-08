Volume fixer
============

The Google Talk plugin will attempt to automatically adjust your mic volume to
a sensible level.  Often this does not work as desired, and ends up setting
your volume far too low or too high.

There used to be an option to disable this feature, but that vanished:

https://productforums.google.com/forum/#!topic/chat/USt57bGcpJc

I wrote this program to work around the problem.  When run, it fetches every
input volume level from PulseAudio, and then watches for changes to those
levels.  Whenever they change, it sets them back to wht they had previously
been.  This prevents the Google Talk plugin (or indeed anything else) from
altering your input volume levels.  Just remember to kill it (with e.g. Ctrl+C)
if you actually do want to alter the volume.

Requires the PulseAudio development headers to compile.  On Debian these can be
found in the libpulse-dev package.

Copyright (C) 2014  John Bytheway (jbytheway at that gmail place)


LICENSE
-------
This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

