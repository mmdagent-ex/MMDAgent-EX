
# Buttons

You can define buttons at each content.  When playing the content, they will appear to the screen by long tap or pressing `q` key.  You can define various content-dependent actions for them.

## How to define a button

Put a button definition file on the top folder of the content package.  The name should be `BUTTON0.txt`, and another button can be defined by `BUTTON1.txt`.  You can defined up to 10 buttons, till `BUTTON9.txt`.

```text
BUTTON0.txt
BUTTON1.txt
...
BUTTON9.txt
```

Furthermore, "sub-buttons" can be defined by placing button definition files as `BUTTON0-0.txt`, `BUTTON0-1.txt` and so on. The sub buttons will not be shown on the screen by default, but appears when their parent button (`BUTTON0.txt` in this case) is pressed.

```text
BUTTON0.txt
BUTTON0-0.txt
BUTTON0-1.txt
BUTTON0-2.txt
BUTTON1.txt
BUTTON1-1.txt
...
```

## Button definition file

Below is a simple example of button definition file.  A line that starts with "`#`" is comment.

```text
#### BUTTON0.txt

image=btn_rocket.png
x=0.2
y=-1.0
scale=1.0
label=Testing
labelX=0.0
labelY=0.0
labelScale=1.0
labelColor=#FF0000
from=left
exec=open,http://www.google.com/
#exec=play,mmdagent://somewhere/some/dir
#exec=message,MODEL_DELETE|mei
#exec=setkeyvalue,__BUTTON1=true
```

### Button image

Image file to be displayed as the button.  Transparent PNG is recommended.

```text
image=imgfile.png
```

### Button position

Center coordinate of the button.

```text
x=1.0
y=-1.0
```

- Positive value is a distance from left and bottom edge of the screen
- Negative value is a distance from right and top edge of the screen

### Button scale

Scale factor of the image.

```text
scale=2.0
```

### Button text

Text label can be displayed on the top of the button image.
Fixed text can be specified by `label`.

```text
label=some string
```

When specifing a key name of KeyValue by preceding the name with "`@`", the value of the key will be displayed as text dynamically.

```text
label=@KeyName
```

### Button text adjustment

The scale and position of the text label can be adjusted by `labelX`, `labelY` and `labelScale`.  The origin of the text is the left-middle edge of the button.

```text
# move text label to upper-right of original and magnify by 1.5
labelX=1.0
labelY=0.5
labelScale=1.5
```

Text color can be specified by `labelColor`.  Color should be like `#RRGGBB` or `#RRGGBBAA`.  Default is `#FFFFFFFF`.

```text
labelColor=#FF0000
```

### Button animation direction

Specify on which edge of the screen the button appears.  Valid values are `left`, `right`, `top`, `bottom` and `parent`.  The `parent` means that the button will appear from the parent button's place, and valid only for sub-button.  Default value is `left`.

```text
from=left
```

### Button actions

An action can be defined to a button by `exec` option.  Several types of actions are supported.

#### 1. Open URL on external browser

To open a Web page, specify action as `open` and the target URL.

```text
exec=open,http://www.google.com/
```

#### 2. Play contents

To start playing a content, specify `play` and the target content.  The target content can be URL (web content) or file path of the .mdf file (local content).

```text
# web contents
exec=play,http://www.google.com/
# local contents
exec=play,xxx/yyy/foo.mdf
```

#### 3. Issue message

Specifying `message` and message string on `exec` to issue the message when the button is pressed.

```text
exec=message,MODEL_DELETE|model
```

#### 4. Set KeyValue

You can also define button to set a KeyValue.  Use `setkeyvalue` and the key-name pair to be set on tap.

```text
exec=setkeyvalue,KeyName=Value
```

### Change button appearance by KeyValue

A button's appearance and actions can be changed at run time.  You can specify a kay-value pair for trigger condition, and properties to be changed when the key-value pair is satisfied.  Below is an example.

```text
#### BUTTON0.txt with variants

## normal button definition
image=...
label=...
exec=...

## variant 1
# condition (required)
1-ifKeyName=KeyName
1-ifKeyValue=Value
# image should be always specified at a variant
1-image=...
# others are optional
1-exec=...
1-scale=...
1-x=...
1-y=...
1-from=...
1-label=...
1-labelX=...
1-labelY=...
1-labelScale=...
1-labelColor=...

## variant 2
2-ifKeyName=KeyName
2-ifKeyValue=AnotherValue
2-image=...
```

The prefix "`1-`" can be "`2-`", "`3-`" and so on to define multiple conditioned button. Up to 9 variants can be defined. When the condition is met, the properties defined as the same prefix will be applied.  Properties not defined with the prefix are kept.
