# Change Log
This is default screen recorder application for Deepin.

## [Unreleased]

## [2.7.3] - 2018-01-25
- Synchronous translations.

## [2.7.2] - 2018-01-25
- Add mkv_framerate and mp4_framerate customize options
- Don't need append char to gif end since gif exit process is correct now, otherwise gif file will break if you stop recording too fast. ;)

## [2.7.1] - 2018-01-25
- Use -vf scale fix "division by 2" error, and now we can use yuv420p pix_fmt to make mp4 play on mobile phone

## [2.7] - 2018-01-25
- Fix tooltip not above on screen-recorder window
- Support flv format recording
- Synchronous translations from transifex.com
- Support deepin-manual newest version
- Fix error that GIF's content is not complete
- Fix GIF file break when stop recording less than 2 seconds
- Fix zero byte mp4 problem

## [2.6.5.1] - 2017-11-22
- Update translation

## [2.6.5] - 2017-11-17
### Bug Fixes
- Bypass window manager
- **flatpak:**  No composite warning dialog
- **hidpi:**  Gif record area calculated wrong

### Features
- Install dbus service ([b1a492f1](b1a492f1))

## [2.6.4] - 2017-11-06
- Add FlatPak support: single instance, keystroke dbus service
- Add composite warning dialog when window manager not support composite
- Fix GIF record area is not correctly under HiDPI
