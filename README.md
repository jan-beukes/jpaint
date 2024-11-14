# JPAINT

## A simple paiting application made with Raylib

### Get the latest version from [Releases](https://github.com/jan-beukes/jpaint/releases)

![](screenshot.png)

## Controls

| Key    | Control      |
| ------ | ------------ |
| b      | brush        |
| e      | eraser       |
| h      | hand         |
| g      | paint bucket |
| c      | clear canvas |
| q      | color select |
| Mouse-scroll | resize brush |
| Ctrl+scroll  | zoom canvas  |
| middle-mouse | pan          |
| L-Alt  | color picker |
| Ctrl+z | Undo         |
| Ctrl+y | Redo         |
| Ctrl+s | Save         |
| Ctrl+o | Open         |
| Ctrl+n | new canvas   |

## Building

Dependencies:

- libGL 
- raylib

Clone jpaint
```
git clone https://github.com/jan-beukes/jpaint.git
cd jpaint
```

raylib and other external dependencies are included in the libs directory

### Linux
```
make
```
### Widows
correctly set the MINGW_CC variable
```
make windows
```


## Features
- [x] Save and load Images
- [x] Create new custom canvas 
- [x] paint bucket
- [x] color picker
- [x] undo/redo
