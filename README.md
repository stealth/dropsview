dropsview
=========

Old school ncurses frontend for reading and sending of _opmsg_ via
_drops_ p2p network.

![screenshot](https://github.com/stealth/dropsview/blob/master/dropsview.jpg)

Nevermind the aggressive color; _dropsview_ runs in skeleton mode which means it
does not set any colors by itself. You can set your own colors or background
images/fading via your terminal emulators' arguments. I just chose the maximum
retro feeling.

Intro
-----

If you are not familar with [opmsg](https://github.com/stealth/opmsg) or
[drops]((https://github.com/stealth/drops), please read their documents first,
as it makes only sense to use dropsview if you have a working _opmsg_ and
_drops_ setup. After all, _dropsview_ is a frontend for these, if you dont want
to use the commandline interface.

It is particular important that you have set up a robust _opmsg_ install where
the personas are properly linked and you dont accidently pump messages into
the _drops_ p2p network signed with personas that you mean to keep secret or so.

Note that personas that you have used in _opmsg_ emails before, are no longer anonymous
since even when used with _drops_ now, they can always be tracked to belong to
that email address that you used in past. The _opmsg_ and _drops_ documents will explain
all this. It is not as difficult as it may sound, you just have to be
aware of these things.


Build
-----

Should be easy: `make`.

You should also install the `qrencode` program, since _dropsview_ can show
the persona key as utf8 QR code, if your `$EDITOR` works with utf8.

Usage
-----

Just start `./dropsview` and it will list you the personas and messages that
you have in your in-queue. Navigate as usual.

Key bindings:

* `f` to refresh the menu listing in order to see newly arrived messages or imported personas.
You may ask why its not automatically refreshed on message arrival. Thats the case so your
workflow is no longer message-driven by others and you can give yourself timeslots for
coding. After such schedule, you decide yourself when to accept new messages, by pressing __f__.
* `v` to switch to visual mode which starts the editor. In personas menu you will see
the public key to make it easier to copy&paste it for import. In message menu, you will
see the decrypted message.
* `b` in message menu to also burn the message after reading in visual mode.
* `q` in personas menu to start editor with utf8 QR code. You should resize
your terminal before, if you want to use this feature. Its meant for quick sharing
of persona keys via smartphone messages. Requires webcam and _zbarimg_ (or similar) on
the other side. Note that you need to verify the persona id afterwards as usual, and
that you may lose anonymity if you send QR codes on channel without end2end encryption.
* `s` in personas menu to send a _opmsg_ to this persona and to submit it to the _drops_ p2p

Messages that have already been read, are marked with a __R__ at the beginning inside
the menu.



