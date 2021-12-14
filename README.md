# ASNS

### Introduction
ASNS - is an ASCII art editor, written in C programming language. You can use it for create / modify some ascii arts, which you can easily destroy in notepad.

### Usage
##### Step 1. clone it
First of all, clone this repo:

    git clone https://github.com/Nikita-bunikido/ASNS.git

##### Step 2. Compile it

Use mingw32-make to compile it:

    cd ASNS\src\
    mingw32-make

##### Step 3. Getting started

For this time you can use it like this, because program is not finished yet:

    asns <w.a. width> <w.a. height>

w.a. - working area.
You can setup your working area using this command.

Use  ```-default``` flag, to set default sizes of working area.

#### Management

| Key | Action |
|:---:|:------:|
| *up arrow* | move cursor up |
| *down arrow* | move cursor down |
| *left arrow* | move cursor left |
| *right arrow*| move cursor right |
| **d**          | start/end drawing mode |
| **b**          | choose new brush |
| **e**          | start/end eraser mode |
| **a**          | save file             |
| **q**          | safe exit without save |