dawlet
=====

Dawlet is a toy DAW (digital audio workstation) whose interface is a webapp.  It's just a proof-of-concept.

When would I use it?
-----

You wouldn't.  You'd use Ardour or Reaper or Bitwig or something.  You could, in theory, record a song in it, but it lacks scrolling, waveform display, editing, plugin support, sends, exporting, MIDI, and no doubt many other things.

How does it work?
-----

There are two threads in the program.  One is the "audio thread", which is the
realtime thread to play, mix, and record audio.  The other thread (called the
"DAW thread" for lack of a better name) does everything else -- it's the
webserver, and it reads and writes from disk.  The two threads communicate
solely by sending messages back and forth (through ring buffers).  The audio
thread is not allowed to do anything that can take unbounded time, and that
includes allocating memory, so the DAW thread needs to keep it supplied with
chunks of memory, usually in the form of audio buffers.  During playback, it
has to read-ahead enough audio from the disk so the audio thread will have it
by the time it needs to play it.

As a webserver, the DAW thread serves up the HTML/JS/CSS, handles a REST-like
API for making changes to the project (such as adding a new track), and handles
transport commands ("play", "pause", "record", etc.) from a Websocket.

Both threads need to access the project data structure, mainly because only the
DAW thread can save it to disk.  The audio thread has the responsibility of
making changes to the project.  Lets consider what happens when a new track is
added to the project.  The DAW thread allocates the Track object, then sends it
to the audio thread (along with information about where to put it).  The audio
thread can make the change quickly when it gets the message (so it doesn't
jeopardize keeping the audio going), but until that happens, it's not safe for
the DAW thread to access the project structure.  So the DAW thread keeps track
of whether it has outstanding "mutation" requests to the audio thread, and will
wait until they've all been performed and acknowledged before it tries to walk
the structure to write it to disk.



