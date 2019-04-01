LibGUI FAQ
----------

Is the libGUI responsible for tft and touch driver init ?
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""

No. This library is not responsible for the driver initialization as the libGUI has no early_init phase.
The task is responsible for early initialize and initialize the TFT and Touch drivers, and associated devices (e.g. SPI bus).

Is the libGUI can print-out icons bigger or smaller than 45x45 ?
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

Not this version. Although, if you which to print splash screens, you can directly call the driver primitive to print out an RLE image. The ili9341 driver support tft_rle_image() API which permit to print an RLE image on the screen.

What is the maximum number of menus ?
"""""""""""""""""""""""""""""""""""""

The maximum number of menus is configureable in the libGUI dediated config entry. A reasonable value should be arround 10

What is the maximum number of tiles ?
"""""""""""""""""""""""""""""""""""""

The maximum number of tiles is configureable in the libGUI dediated config entry. A reasonable value should be arround 30. Remeber that a tile structure is big and increasing the number of allowed tiles may impact the memory size of the generated application.
