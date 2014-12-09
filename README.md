MrsWatson
=========

MrsWatson is a command-line audio plugin host. It takes an audio and/or MIDI
file as input, and processes it through one or more audio plugins. Currently
MrsWatson only supports VST 2.x plugins, but more formats are planned in the
future. MrsWatson was designed for primarily three purposes:

* Audio plugin development and testing
* Automated audio processing for servers or other applications
* Unit testing audio plugins


Examples
--------

Say you have an audio file which you would like processed through a group of
mastering plugins.

    mrswatson --input mysong.pcm --output out.pcm --plugin plugin1;plugin2

This will print the following output:

    - 00000000 000000 Plugin 'plugin1' is of type VST2.x
    - 00000000 000000 Plugin 'plugin2' is of type VST2.x
    - 00000000 000000 MrsWatson version 0.1.0 initialized
    - 00000000 000000 Opening VST2.x plugin 'plugin1'
    - 00000000 000029 Opening VST2.x plugin 'plugin2'
    - 00000000 000144 Processing with samplerate 44100, blocksize 512, 2 channels
    - 03532800 000204 Total processing time 60ms, approximate breakdown by component:
    - 03532800 000204   plugin1: 38ms
    - 03532800 000204   plugin2: 11ms
    - 03532800 000204   MrsWatson: 11ms
    - 03532800 000204 Read 3528000 frames from mysong.pcm, wrote 3532800 frames to out.pcm
    - 03532800 000204 Shutting down
    - 03532800 000210 Closing plugin 'plugin1'
    - 03532800 000212 Closing plugin 'plugin2'
    - 03532800 000212 Goodbye!

To see more or less logging output, use the `--verbose` or `--quiet` options,
respectively. MrsWatson generates colored output (if your terminal supports
it) with two times per line, the first for the current sample and the second
for the time in milliseconds since processing began. The sample time also
changes colors after every 44100 samples to help visually break up processing
times. This value can be changed with the `--zebra-size`  option.

To process a MIDI file through an instrument, you'd do something like this:

    mrswatson --midi-file mysong.mid --output out.pcm --plugin piano,soft.fxp

Like the first example, a list of plugins separated with semi-colons can be
given here so that the audio generated by the instrument can be processed
through any number of effects. Each plugin name can be followed by a comma and
the location to a preset file to be loaded before processing.

Complete help for MrsWatson can be found by running the program with no
arguments, or with `--help`. The `--help` switch prints quite a lot of output,
so you can also use `--options` to see all supported options and their default
values.


Loading Plugins
---------------

Currently, MrsWatson loads plugins by their short name by searching in the
standard installation locations for your platform, as well as the current
working directory and by absolute path. Use the `--list-plugins` option to see
the order of locations searched and the plugins found there.

While MrsWatson has some understanding of getting and setting plugin
parameters, it does not extend this functionality to the end user. Rather than
passing a huge list of parameters on the command line, one should instead
create a preset for the plugin and load it with the comma-separated syntax
as shown above.


Limitations
-----------

As MrsWatson is a new codebase, there are lots of missing features, some more
important than others. To encourage a quick initial release, the following
features are not yet present in MrsWatson, but may be added at some point in
the future:

* AudioUnit plugins on Mac OSX
* File support for compressed audio
* Resampling of input source if desired
* True realtime mode

I have also tried to identify incomplete areas of the code and log them to the
console, which means that we are aware that this feature is missing and will
be added soon. If you see some other missing functionality or experience a
crash or other bug, please report an issue on the [project page][3].


Bug Reporting
-------------

If you believe you have found a bug in MrsWatson, please try first running it
with the `--verbose` argument. This will generate extra logging output which
may help to solve the problem.

The easiest way to report a bug is to send an email to Teragon Audio's support
address: support (at) teragonaudio (dot) com. MrsWatson has a special
command-line switch to aid in diagnosing runtime problems, `--error-report`.
When enabled it will create a zipfile on the desktop containing the input,
output, logs, and optionally the plugins themselves. Please include these
reports for bugs resulting in incorrect behavior or crashes.

A test suite program, named `mrswatsontest`, can be found in the Mrswatson 
zipfile. If tests fail on your platform, please report this along with your
bug.

MrsWatson uses [GitHub issues][4] for bug reporting, if you would like to
submit an issue yourself.


Building
--------

Instructions for building MrsWatson can be found in the file
[Building.txt][9].


History and Name
----------------

MrsWatson takes its name from a prior product from [Teragon Audio][1] called
*MissWatson* (and the original name there, in case you were wondering, is a
bit of a joke on the [Dr. Watson][2] utility).

In 2009 I sold the exclusive rights to MissWatson to a company interested in
using it for server-side audio processing. As per the terms of our agreement,
I made one last public release, and removed the code from my website. Since
then, I have received numerous emails and inquiries regarding the project,
mostly from plugin developers or people interested in VST technology. Though I
have no regrets about discontinuing the original MissWatson (particularly
since I didn't have time to properly maintain it), I felt that it was a useful
utility which should be made available and open-sourced.

In 2011, my NDA with the aforementioned company expired, and I started working
on a new MissWatson. MrsWatson is the result of this labor.

As I no longer have the source code for MissWatson, MrsWatson is a black-box
implementation. They share only the name and concept in common. The two
programs are **incompatible** and have **fundamentally different behavior**.
They are also written in different languages (the original in C++, MrsWatson
in C), and have completely different internal architecture.


Donate
------

If you are using MrsWatson to do something cool, please send me a link to your
project! If you appreciate MrsWatson and would like to donate money, please
instead make a donation to a charity on our behalf, and let us know about it.
The organizations which have helped us the most are:

* [EFF][5]: Without the EFF, programs like MrsWatson would be significantly
  harder to create and distribute.
* [Wikipedia][6]: Writing MrsWatson involves a lot of research as well as
  coding, and Wikipedia is an essential part of this.


Special Thanks
--------------

Big additional thanks to:

* Andrew McCrea, (@thrusong)
* Michael Pruett, (@mpruett)
* Jarl Friis (@jarl-dk)


Licensing
---------

MrsWatson is made available under the BSD license. For more details, see the
`LICENSE.txt` file distributed with the source code. MrsWatson also uses the
following third-party libraries, which are licensed under the respective
agreements:

* [VST][7]: Licensed under Steinberg's VST SDK license agreement, version 2.4.
  For more information, see Steinberg's developer portal.
* [libaudiofile][8]: Written by Michael Pruett, licensed under GNU Library
  General Public License.

[1]: http://www.teragonaudio.com
[2]: http://en.wikipedia.org/wiki/Dr._Watson_(debugger)
[3]: http://github.com/teragonaudio/MrsWatson
[4]: https://github.com/teragonaudio/MrsWatson/issues
[5]: https://supporters.eff.org/donate
[6]: http://wikimediafoundation.org/wiki/WMFJA085/en
[7]: http://www.steinberg.net/en/company/developer.html
[8]: http://audiofile.68k.org/
[9]: https://github.com/teragonaudio/MrsWatson/blob/master/doc/Building.md
