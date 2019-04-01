About the GUI
-------------

Principles
""""""""""

The graphical user interface is based on a succession of menus and tiles declaration.

   * Each menu is independent and can be accessed from a tile touch.
   * Each tile is associated to a given menu and has various properties

      * A width
      * A height
      * A background and a foreground color
      * A text content
      * An icon
      * An associated action

Each GUI element (menu or tile) is associated to a *descriptor*, which is returned at the element declaration. This descriptor is used during the lifetime
of the element to update it (e.g. adding a tile to a given menu descriptor, or
updating a tile content).

The tile positioning in a menu is automatically done, from top to bottom of
the screen, from left to right, depending the each successive declared tile
size of a given menu.

For example, if you declare, for a given menu:

   * A tile of size (TILE_WIDTH_FULL, TILE_HEIGHT_STD)
   * A tile of size (TILE_WIDTH_HALF, TILE_HEIGHT_STD)
   * A tile of size (TILE_WIDTH_HALF, TILE_HEIGHT_STD)
   * A Tile of size (TILE_WIDTH_FULL, TILE_HEIGHT_DOUBLE)

The resulting menu would look like:

resulting menu would look like:

.. raw:: html

   <center><pre>
   +-----------+
   |           |
   |  tile 1   |
   +-----+-----+
   |     |     |
   | t 2 | t 3 |
   +-----+-----+
   |           |
   |  tile 4   |
   |           |
   |           |
   +-----------+
   </center></pre>


.. caution::
   By now, reduced sized tiles (for e.g. WIDTH_HALF or WIDTH_THIRD) are supported only for TILE_HEIGHT_STD height)
