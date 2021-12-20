# screenkey-guard

A program to keep your passwords safe while
using screenkey in front of an audience.

## What it does
On startup this program launches screenkey
(alternatively a PID of an existing screenkey can be provided)
and then proceeds to await for the appearance of programs that have "sudo" or "pinentry" in their name.

Whenver it discovers a program that has the aforementioned qualities, it kills the existing `screenkey`-program.

After the password-asking-programs have exited, the guard then relaunches screenkey.
