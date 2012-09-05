Luiji's Tiny BASIC Compiler
===========================

TBC is a compiler for the Tiny BASIC programming language, common on older
microcomputers due to the original interpreter's tiny size (3K). I programmed
this as a self-tutorial on how to write compilers and as a basis for a future
BASIC compiler I plan on writing for the Arduino microcontroller. There are
certain areas where TBC strays from the canonical Tiny BASIC implementation,
however, mainly derived from the compiler's esoteric basis and certain
practical limitations.

Limitations
-----------

* You cannot place arbitrary spaces in keywords. `GO TO <> GOTO`.
* You cannot jump to calculated positions; `GOTO` only accepts plain integers.
* The `CLEAR` and `LIST` directives are not implemented because they have no
  place in this type of compiler.

Platforms
---------

The primary target of TBC is Netwide Assembler that can be compiled into
MS-DOS, FreeDOS or DosBox COM executables. Support for other platforms may
come in the future.

TBC is written in forwards-compatible K&R C using only the `getopt` extension.
It should be compilable with the vast majority of C and C++ compilers. We
provide a copy of BSD's `getopt` implementation (`getopt.c`) for situations
where a native version is not available. For more information, see
**Installation**.

Installation
------------

No build scripts are provided because, in my opinion, TBC is simple enough to
lack them. On most systems you will probably use the GNU Compiler Collection,
with the exception of Windows where there's a high chance you will want to use
the Microsoft Visual C++ Compiler.

TBC has been written in forwards-compatible K&R C which means that it can be
compiled with Bruce's C Compiler for embedded Linux systems if you wanted that
(unlikely). For compilers that lack an implementation of `getopt` (i.e. MSVC),
you will have to include `getopt.c` in the list of source files.

With the GNU Compiler Collection,

        gcc tbc.c -o tbc

With the Microsoft Visual C++ Compiler,

        cl tbc.c getopt.c

With Bruce's C Compiler,

        bcc tbc.c -o tbc

To target i386 Linux or MS-DOS pass `-Mg` and `-Md` to Bruce, respectively.
The majority of alternative compilers should function in a similar fashion.
Actual installation is as simple as putting the resulting executable (`tbc` or
`tbc.exe`) somewhere in your system's program search path.

Syntax Highlighting
-------------------

I provide a GtkSourceView-3.0 script for GEdit in `tinybasic.lang`. Copy this
into `/usr/share/gtksourceview-3.0/language-specs` and restart GEdit to start
highlighting `.bas` files in TinyBASIC. `/usr/local/share` and
`/home/you/.local/share` are valid alternatives to `/usr/share` in most
configurations.

As far as I can tell, ViM already supports TinyBASIC syntax. Feel free to send
me syntax definitions for other text editors and I might include them with the
compiler (with appropriate credit, of course).

Legal
-----

TBC is licensed under the GNU General Public License version 3 or later. A
slightly-modified copy of BSD's `getopt` implementation is provided for
platforms that don't have native `getopt` implementations (Windows) and is
licensed under the 3-Clause BSD license.

## Luiji's Tiny BASIC Compiler

Copyright (C) 2012 Entertaining Software, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

(For full license see `COPYING`.)

## getopt.c

Copyright (C) 1987-2002 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

A. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

B. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

C. Neither the names of the copyright holders nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
