# copyapt

**copyapt** causes **apt** to install software on one system to be
installed on another system.



## SYNOPSIS

**copyapt** options *aptsource.lst* *aptdest.lst*

From a source computer (or from backup) generate *aptsource.lst*:

**apt** list --installed > *aptsource.lst*

At the newly installed computer (or computer out of date with
what is installed):

**apt** list --installed > *aptdest.lst*

then:

**copyapt** --cut-at \the_version *aptsource.lst* *aptdest.lst*

Program will leave package names in *aptsource.lst* and *aptdest.lst*
with the extraneous data in each line cut off from *the_version* on.

Next, it will remove any line in *aptsource.lst* that exists in
*aptdest.lst*.

Finally it will invoke **apt** to install whatever remains in
*aptsource.lst*.

There will be many reported errors as it trys to install numerous
libraries that are already installed as a consequence of installing
main packages. This is harmless.

The program must be run as root or **apt** will fail.

## EXAMPLE
**copyapt** --cut-at /xenial *aptsource.lst* *aptdest.lst*
