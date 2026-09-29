#ifndef DUMP_EDID_H
#define DUMP_EDID_H

#include <exec/types.h>

#define EDID_BLOCK_SIZE   (128)
#define EDID_BLOCK_COUNT  (256)

ULONG Get_EDID_Primary(UBYTE * buffer);
ULONG Get_EDID_Display(ULONG display, UBYTE * buffer);

#endif /* DUMP_EDID_H */
