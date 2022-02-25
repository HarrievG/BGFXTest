ExportFontToDoom3.
Version: 1.02.

A command-line application that exports standard fonts into Doom 3's font format.

Grant Davies, 2005.
<g.davies@optusnet.com.au>


Acknowledgements
================

This application makes use of the FreeType font library.
http://www.freetype.org

This application makes use of the DevIL image library.
http://openil.sourceforge.net


Usage
=====

To export with the common settings of 12, 24, and 48 point fonts, simply type:

ExportFontToDoom3 <font file name>

eg. ExportFontToDoom3 times.ttf

Most standard font formats can be used for export.


	-size <size>

Export a certain size.  More than one "-size" arguments may be specified.

ExportFontToDoom3 <font file name> [-size <font point size>]

eg. ExportFontToDoom3 times.ttf -size 12 -size 24 -size 48

If no "-size" argument is specified, 12, 24, and 48 point fonts are exported.


	-xOffsetFix <type>

As a result of the limitation in Doom 3's font renderer outlined below (in 
"Known limitations : The Doom 3 X offset issue."), one of two fixes can be applied, 
each with its own drawback.  These fixes are also outlined in the section below.

The types are: none, glyph (modifies the glyphs but does not fully correct
glyphs with negative x offsets), and font (modifies all glyphs but will render at a
slightly offset location on the x axis - this is the default).

If "none" is specified, the X offset value is exported to the "pitch" member of the
"glyphInfo_t" structure in Doom 3.  Using Doom 3's default font renderer will render
data exported using this type very poorly.  However, using this type allows changes
to be made to the Doom 3 SDK that will render the font perfectly.  Code to render
a glyph contains a line similar to the following:

	fYPosition -= k_rGlyph.top * fScale;

If another line is added to the rendering algorithm along the lines of the following:

	fXPosition += k_rGlyph.pitch * fScale;

Then the in-game rendered text will render perfectly.  However, GUI text will still
render poorly if exported under this mode since the GUI font rendering code is not
exposed in the Doom 3 SDK.  Therefore, it may be wise to have 2 exports of the font:
one for the in-game rendering with the code changes and exported using the "none" fix
mode, and another which is exported as dds files using the "font" fix mode for the GUI 
font rendering.

If "font" is specified, the amount of pixels that the font has been offset by is exported
to the "pitch" member of the "glyphInfo_t" structure of every glyph in Doom 3.  This
may help to re-align text.

eg. ExportFontToDoom3 times.ttf -xOffsetFix none
eg. ExportFontToDoom3 times.ttf -xOffsetFix glyph
eg. ExportFontToDoom3 times.ttf -xOffsetFix font


	-noXOffsetWarnings

Suppress warnings associated with X offset issue.  This defaults to off (i.e.
warnings are on).


	-textureFormat <format>

Specifies the type of texture file to output.  Common values are "tga" and "dds".
Generally, "tga" will be desired, however, because of a limitation with GUI rendering
in the current version of Doom 3, it may be required to output "dds" textures.

NOTE: Current dds does not seem to work as expected.  This may be addressed in a later
version, time permitting.  Until then, use a tga to dds conversion program.

If no texture format is specified, the default is tga.


	-name <name>

Names the font to be exported.  By default, the name is extracted from the font file.


Output
======

A folder will be created and all output files will be stored in this new folder.
The folder will be named according to the font being exported.

The output will consist of targa image (.tga) files and .dat files.  These files
need to be copied into your mod folder in "fonts/english/<your font name>"


Known limitations : The Doom 3 X offset issue.
==============================================

Most font renderers support an x and y offset for each glyph in a font.

In the case of the y offset, this is so that glyphs that are tall ('h') or that extend below
the baseline ('g') will line up correctly with other glyphs.  Doom 3 supports this.

In the case of the x offset, this is so that glyphs that ought to be overlaid on
top of other glyphs (such as 'j') are drawn correctly.  However, Doom 3 does not support this.

There are two cases for the x offset - a positive and negative x offset.

In the case of a positive x offset, the exporter can automatically pad out the glyph with
blank space to emulate the x offset.

In the case of a negative x offset, there is no perfect workaround.  This is why
'j' tends to render poorly in some fonts when q3font is used.

There are two fixes for this issue, each with its own drawback.

The first is to pad out glyphs with a negative x offset, which means the
rendering of these glyphs will not look quite right (in general, a handful of glyphs
in each font use a negative x offset, and the exporter will output the exact number).
To enable this fix, use the "-xOffsetFix glyph" command-line switch.

The second is to pad out every glyph in the font with an offset.  Glyphs with
a negative x offset will render correctly under this scheme, however the entire font
will be offset in the positive x direction.  The exporter will output the amount that
the font is offset.  This offset will need to be considered when positioning strings.
To enable this fix, use the "-xOffsetFix font" command-line switch.

Note that there is a hybrid fix as described above, by exporting two versions of the
font with a modified Doom 3 SDK to fix the x offset rendering issue.  However, even
under this system the GUI-rendered text will still suffer from one of the problems
outlined in the above paragraphs.


Known issues : DDS support.
===========================

Currently, DDS output will probably not work as expected.  Use a garden variety
tga to dds conversion application until it is fixed.


Contact
=======

Bugs/comments/feedback: Grant Davies <g.davies@optusnet.com.au>
