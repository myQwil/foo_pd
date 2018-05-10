# foo_pd
a plugin for foobar2000 that allows you to play and interact with Pure Data files (.pd)

## Features
* reads/writes metadata to/from patches. This is done by storing the info in the form of comments, in a canvas called [pd meta] or [pd info]. If no such canvas exists, foobar will add it in the top left-hand corner of your patch.

* comes with a Win32 Dialog UI element, containing sliders, toggles, buttons, and edit-text/button combos to send messages to your patch.
  * foobar looks in your patch for a canvas called [pd mix] and uses the parameters of whatever sliders, bang objects, or toggles it finds there to give your UI controls similar functionality.

## How playback works
* libpd sends a 1 to vol, then a bang to play.
  * Generally this is where, in your patch, you have an [r play] hooked up to your metro, and an [r vol] connected to a [*~ ] before your output reaches [dac~].

* the length of the song is arbitrarily set by the user.
  * This affects the trackbar's cursor visibility and ability to set a position.
  * If the length is 0, there will be no trackbar cursor. Otherwise, the cursor, when moved, will send its position in seconds to 'pos'. From there, it's up to your patch to take that information from [r pos] and work it into song events.
  * The patch will not actually stop and move on to the next track until libpd receives a bang from [s stop].

## How the mixer works
* all mixer controls go inside of [pd mix]

* horizontal and vertical sliders are turned into slider controls in the UI element
  * labels assigned to sliders in the patch become labels for the UI element's slider controls. The same applies for send symbols.
  * min and max values of sliders on the UI element work in integers only, so if you want a gradual shift from, say, 0 to 1, write "gradient" in the slider's receive symbol, and the slider's range will be broken down into roughly 200+ individual steps.
  * there are currently 7 sliders in the UI element

* bang objects with no label become simple buttons in the UI
  * their send symbols will be reflected in the button's name and they will send a bang when clicked.
  * there are currently 3 buttons

* bang objects with a label assigned become message buttons
  * these have an edit text field associated with them, where you can type out any message you want and send it to the destination.
  * the bang's label is placed inside of the edit text field as a suggested message to send.
    * pure data strips commas out of labels, so I'm using apostrophes to denote where commas should go.
    * example: do this' then this
  * there are currently 2 message buttons, with the 'any' button being a potential 3rd.

* a bang object with a label written in the format "dest:msg" will be assigned to the 'any' button.
  * the 'any' button has an editable destination field, giving you access to basically any receive symbol in your patch.
  * also substitutes as a third normal message button, when the other two are already in use

* toggles become checkboxes
  * each checkbox can have a label and send symbol assigned to it
  * there are currently 4 checkboxes

* right-clicking a track shows the context menu entry Pd Player -> Load mixer.
  * basically, you can load mixers of tracks not currently playing for some potentially interesting exchanges between patches. After loading the mixer, you still need to hit the Refresh button to show the changes.

This plugin uses libpd. If you want your own patches to work with foo_pd, you might need to make another build of libpd.dll using MSYS2. If you're not sure which objects aren't instantiating, foobar's console prints all of pd's messages while audio is being processed.
