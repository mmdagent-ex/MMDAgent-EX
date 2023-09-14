# Full list of what's new from MMDAgent-EX 1.3 to 2.0

## Android / iOS support

- Many bug fixes
- Android 5.0 is now not supported, use 6.0 and later

## UI cleanup

- Re-organized UI with tab bar, menu
- No more edge flicking
- Clearer font

## Camera move by touch disabled by default

Now camera will not move on touch by default.

- Allow camera move: Key 'c' or menu
- Reset camera position: Key 'Shift+C' or menu
- Show help text on screen at camera move


## New .mdf options

Two options are added.

- Now Buttons will be shown by default.  You can specify whether to show buttons at start up (default is true)

```text
show_button=false
```

- Specify whether to show text captions at start up (default is true)

```text
show_caption = false
```

## Time message

`CURRENT_TIME` will be issued to tell the current time (hour and minutes) at every 30 seconds:

```text
CURRENT_TIME|hh|mm
```

## Tapped message

`TAPPED` will be issued on every screen tap or click with screen position.

```text
TAPPED|x|y
```

## In-content Buttons

In addition to static "Buttons" defined in package's BUTTON*.txt, you can now define and display a simple button in the dislogue scenario for simple, on-demand use.

Define button function and show the image on screen:

```text
BUTTON_ADD|alias|scale|x,y|image path|action|(ON or OFF for autoclose)
   scale = display scale (0.0-),
   x, y = coordinate from button left (negative value is translated as upper, right)
   image path = path to image file
   action = action when tapped, as same as BUTTON*.txt (play|open|message|keyvalueset|...)
   autoclose (ON|OFF): ON to delete this button after one tap, or OFF to keep showing.
```

Delete the button:

```text
BUTTON_DELETE|alias
```

Events to be issued:

```text
BUTTON_EVENT_ADD|alias
   will be issued when the button becomes shown and active on screen.
BUTTON_EVENT_EXEC|alias
   will be issued when the button was tapped.
BUTTON_EVENT_DELETE|alias
   will be issued when the button was deleted by tap or BUTTON_DELETE.
```

## Motion blending

When overlaying multiple motions (either full body or part of body) at a model, the default behavior is "replace", in that the motion of the higher priority will supercede other motions.  However, you can use `MOTION_CONFIGURE` message to change the behavior per bone basis.

```text
MOTION_CONFIGURE|(model alias)|(motion alias)|MODE_BONE_REPLACE|bonename[,bonename,..]
MOTION_CONFIGURE|(model alias)|(motion alias)|MODE_BONE_ADD|bonename[,bonename,..]
MOTION_CONFIGURE|(model alias)|(motion alias)|MODE_BONE_NONE|bonename[,bonename,..]
MOTION_CONFIGURE|(model alias)|(motion alias)|MODE_FACE_REPLACE|facename[,facename,..]
MOTION_CONFIGURE|(model alias)|(motion alias)|MODE_FACE_ADD|facename[,facename,..]
MOTION_CONFIGURE|(model alias)|(motion alias)|MODE_FACE_NONE|bonename[,bonename,..]
```

From 2.0, we have added new per-motion blending configurations for simpler use.  Also, you can set blending rate for each motion.  The default is "replace", with blend rate of 1.0.

```text
MOTION_CONFIGURE|(model)|(motion)|MODE_REPLACE
    set motion overlay behavior to "replace"
MOTION_CONFIGURE|(model)|(motion)|MODE_REPLACE|(rate)
    set motion overlay behavior to "replace", and set the motion blend rate of this motion (0.0-1.0)
MOTION_CONFIGURE|(model)|(motion)|MODE_ADD
    set motion overlay behavior to "add"
MOTION_CONFIGURE|(model)|(motion)|MODE_ADD|(rate)
    set motion overlay behavior to "add", and set the motion blend rate of this motion (0.0-1.0)
MOTION_CONFIGURE|(model)|(motion)|BLEND_RATE|(rate)
    set the motion blend rate of the motion (0.0-1.0)
```

Motion blending algorithm when computing current pose from motion at each frame:

```text
- Reset bone parameters
- For all motions [m] in priority order from low to high:
   - compute pose specified by [m]
   - Multiply the pose by the blend rate of [m]
   - If ([m].mode == replace)
         apply (set) the pose to each bone parameters
     else if ([m].mode == add)
         add the pose to the each bone parameters
```

## FST extensions

.fst format was extended for easier use.

### Block definition

You may be bored of writing straight state sequences with unique numbers like this...

```text
0 10   A      B
10 20  <eps>  C
20 30  EVENT  COMMAND1
30 40  <eps>  COMMAND2
```

From 2.0 you can write the same straight state sequence in a simpler form like this.
Place "`:`" after state numbers, then write only the in-out pairs as **indented** lines.
The indentation depth can be any.

```text
0 40:
    A      B
    <eps>  C
    EVENT  COMMAND1
    <eps>  COMMAND2
```

Furthermore, you can also write an "OR" arc in simpler form.  You may experience writing several arcs on the same in-state and out-state to capture alternative messages like this:

```text
#  at state 0 we should capture either A or B to issue X and proceed to 10
0 10   A      X
0 10   B      X
10 20  EVENT  COMMAND1
20 30  <eps>  COMMAND2
```

From 2.0, you can write the same sequence by using "`+`" inside a block definition.

```text
0 30:
    A      X
    +B     X
    EVENT  COMMAND1
    <eps>  COMMAND2
```

Internally, the "`+`" definition will not define a new state but append parallel arc to the previous one.

More, if the output of the "`+`" line is the same, you can omit it.

```text
0 30:
    A      X
    +B
    EVENT  COMMAND1
    <eps>  COMMAND2
```

### Initialize local variables

You can write initialization of local variables at the top of each .fst file.  It will be evaluated at FST load time.

```text
# initial values
${agentPMD}="Agents/mai/mai.pmd"
${camera_default}="1.7,12.7,0.0|0.0,0.0,0.0|44|16|1"

# can not set after the beginning of any state definition
0 20:
    <eps> STAGE|Stage/floor.png,Stage/clouds.jpg
    ...
```

### Local variable test

The 3rd field is normally a input text matching condition, but from 2.0 you can write a special condition that tests a local variable:

```text
10 20  ${flag}==xxx  MODEL_ADD|mei|...
```

In this case, the local variable `${flag}` will be tested when comes to state 10.  If the value is `xxx`, the `MODEL_ADD|mei|...` will be issued.

Only `!=` and `==` are available (local variables are strings), and no space is allowed in the equation.

```text
10 20  ${flag}==xxx  MODEL_ADD|mei|...
10 20  ${flag}!=yyy  MODEL_ADD|mei|...
```

Please be aware of evaluation timing: the test will be evaluated at the first arrival of the state, and **at every message arrival**, not one-shot or busy wait.

### Access to KeyValue inside FST

You can now get/set a KeyValue string value using local variable by specifying the key name with "`%`".


```text
      # get KeyValue to local variable:
      ${varname}=${%KeyName}

      # set KeyValue from value or local variable:
      ${%KeyName}=string
      ${%KeyName}=${varname}
```

### Example

```text
## set initial values of local variable
${agentPMD}="Agents/mai/mai.pmd"
${camera_default}="1.7,12.7,0.0|0.0,0.0,0.0|44|16|1"

## get definition of "current_time=..." in .mdf file
${time}=${%current_time}

## block definition, use local variable
0 10:
    <eps> STAGE|Stage/floor.png,Stage/clouds.jpg
    <eps> MODEL_ADD|mei|${agentPMD}
    MODEL_EVENT_ADD|mei CAMERA|${camera_default}

## OR description in block
10 20:
   KEY|1 <eps>
   +KEY|2
   +MESSAGE|PROCEED
   <eps>  SYNTH_START|mei|normal|"Hi!"
   SYNTH_EVENT_START|mei|normal <eps>

## variable test
#
# Before 1.3 (still works in 2.0)
#20 25 ${time}==morning SYNTH_START|mei|normal|"Good morning!"
#20 25 ${time}==evening SYNTH_START|mei|normal|"Good evening!"
#25 30 SYNTH_EVENT_START|mei|normal <eps>
#
20 30:
   ${time}==morning SYNTH_START|mei|normal|"Good morning!"
   +${time}==evening SYNTH_START|mei|normal|"Good evening!"
   SYNTH_EVENT_START|mei|normal <eps>
```

## VSCode mode for .fst

Tired of writing scenario?  We now offers an experimental VSCode extention for editing .fst file.  See `Tools/vscode-extension/install.txt` how to install it.

- Colors and Highlights
- Message name auto completion
- "Peek / Jump to Definition" on state id
- "Jump to Reference / Show List" on state id

----------

ドキュメント更新
FSTのローカル変数はすべて文字列。数字ではない。
typeとargをまたいだローカル変数は定義できない
ローカル変数で使える文字
All .mdf options をトップレベルへ
Writing scenario に .fst をトップレベルへ
Using KeyValue を HowTo へ移動
