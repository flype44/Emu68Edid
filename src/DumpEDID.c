/******************************************************************************
 * 
 * DumpEDID.c
 * 
 *****************************************************************************/

#include <dos/dos.h>
#include <exec/exec.h>
#include <resources/mailbox.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/mailbox.h>

#include "LE32.h"
#include "DumpEDID.h"

extern APTR MailBoxBase;

#define TAG_GET_NUM_DISPLAY         (0x00040013)
#define TAG_GET_DISPLAY_ID          (0x00040016)
#define TAG_GET_EDID_BLOCK          (0x00030020)
#define TAG_GET_EDID_BLOCK_DISPLAY  (0x00030023)

/******************************************************************************
 * 
 * Checksum()
 * 
 ******************************************************************************/

STATIC UBYTE Checksum(UBYTE * buffer, ULONG size)
{
	UBYTE sum = 0;
	
	while (size--)
	{
		sum += *buffer++;
	}
	
	return sum;
}

/******************************************************************************
 * 
 * Get_Num_Display()
 * 
 *****************************************************************************/

typedef struct {
    ULONG  size;
    ULONG  code;
    ULONG  tag_id;
    ULONG  tag_size;
    ULONG  tag_code;
    ULONG  tag_num_display;
    ULONG  tag_end;
} get_num_display_t;

STATIC LONG Get_Num_Display(VOID)
{
	STATIC get_num_display_t cmd;
	
	cmd.size            = sizeof(get_num_display_t);
	cmd.code            = 0;
    cmd.tag_id          = TAG_GET_NUM_DISPLAY;
    cmd.tag_size        = 4;
    cmd.tag_code        = 0;
    cmd.tag_num_display = 0;
    cmd.tag_end         = 0;
	
	MB_RawCommand((ULONG *)&cmd);
	
	if (cmd.code == MB_SUCCESS)
	{
		return (LONG)cmd.tag_num_display;
	}
	
	return (-1);
}

/******************************************************************************
 * 
 * Get_Display_ID()
 * 
 *****************************************************************************/

typedef struct {
    ULONG  size;
    ULONG  code;
    ULONG  tag_id;
    ULONG  tag_size;
    ULONG  tag_code;
    ULONG  tag_display_id;
    ULONG  tag_end;
} get_display_id_t;

STATIC LONG Get_Display_ID(ULONG display_num)
{
	STATIC get_display_id_t cmd;
	
	cmd.size           = sizeof(get_display_id_t);
	cmd.code           = 0;
    cmd.tag_id         = TAG_GET_DISPLAY_ID;
    cmd.tag_size       = 4;
    cmd.tag_code       = 0;
    cmd.tag_display_id = display_num;
    cmd.tag_end        = 0;
	
	MB_RawCommand((ULONG *)&cmd);
	
	if (cmd.code == MB_SUCCESS)
	{
		return (LONG)cmd.tag_display_id;
	}
	
	return (-1);
}

/******************************************************************************
 * 
 * Get_EDID_Block()
 * 
 *****************************************************************************/

typedef struct {
    ULONG  size;
    ULONG  code;
    ULONG  tag_id;
    ULONG  tag_size;
    ULONG  tag_code;
    ULONG  tag_block;
    ULONG  tag_status;
    UBYTE  tag_edid[EDID_BLOCK_SIZE];
    ULONG  tag_end;
} get_edid_block_t;

STATIC BOOL Get_EDID_Block(ULONG block, UBYTE * buffer)
{
	STATIC get_edid_block_t cmd;
	
	cmd.size       = sizeof(get_edid_block_t);
	cmd.code       = 0;
    cmd.tag_id     = TAG_GET_EDID_BLOCK;
    cmd.tag_size   = EDID_BLOCK_SIZE + 8;
    cmd.tag_code   = 0;
    cmd.tag_block  = block;
    cmd.tag_status = 0;
    cmd.tag_end    = 0;
	
	MB_RawCommand((ULONG *)&cmd);
	
	if ((cmd.code == MB_SUCCESS) && (cmd.tag_code == 0x80000088))
	{
		if (buffer != NULL)
		{
			CopyMem(&cmd.tag_edid, buffer, EDID_BLOCK_SIZE);
			Swap32(buffer, EDID_BLOCK_SIZE);
			return (TRUE);
		}
	}
	
	return (FALSE);
}

/******************************************************************************
 * 
 * Get_EDID_Block_Display()
 * 
 *****************************************************************************/

typedef struct {
    ULONG  size;
    ULONG  code;
    ULONG  tag_id;
    ULONG  tag_size;
    ULONG  tag_code;
    ULONG  tag_block;
    ULONG  tag_display_id; // eg. Get_Display_Id()
    UBYTE  tag_edid[EDID_BLOCK_SIZE];
    ULONG  tag_end;
} get_edid_block_display_t;

STATIC BOOL Get_EDID_Block_Display(ULONG display_id, ULONG block, UBYTE * buffer)
{
	STATIC get_edid_block_display_t cmd;
	
	cmd.size           = sizeof(get_edid_block_display_t);
	cmd.code           = 0;
    cmd.tag_id         = TAG_GET_EDID_BLOCK_DISPLAY;
    cmd.tag_size       = EDID_BLOCK_SIZE + 8;
    cmd.tag_code       = 0;
    cmd.tag_block      = block;
    cmd.tag_display_id = display_id;
    cmd.tag_end        = 0;
	
	MB_RawCommand((ULONG *)&cmd);
	
	if ((cmd.code == MB_SUCCESS) && (cmd.tag_code == 0x80000088))
	{
		if (buffer != NULL)
		{
			CopyMem(&cmd.tag_edid, buffer, EDID_BLOCK_SIZE);
			Swap32(buffer, EDID_BLOCK_SIZE);
			return (TRUE);
		}
	}
	
	return (FALSE);
}

/******************************************************************************
 * 
 * Get_EDID_Primary()
 * 
 ******************************************************************************/

ULONG Get_EDID_Primary(UBYTE * buffer)
{
	UBYTE * buffer_start = buffer;
	ULONG block = 0;
	
	/* obtain edid data from primary display */
	if (Get_EDID_Block(block, buffer))
	{
		if (Checksum(buffer, EDID_BLOCK_SIZE) == 0)
		{
			UBYTE extensions = buffer[126];
			
			buffer += EDID_BLOCK_SIZE;
			
			/* obtain edid extensions if any */
			if (extensions > 0 && extensions < EDID_BLOCK_COUNT)
			{
				for (block = 1; block <= extensions; block++)
				{
					if (Get_EDID_Block(block, buffer))
					{
						buffer += EDID_BLOCK_SIZE;
					}
					else
					{
						break;
					}
				}
			}
		}
		else
		{
			PutStr("Bad checksum\n");
		}
	}
	
	/* return edid size */
	return (ULONG)(buffer - buffer_start);
}

/******************************************************************************
 * 
 * Get_EDID_Display()
 * 
 ******************************************************************************/

ULONG Get_EDID_Display(ULONG display, UBYTE * buffer)
{
	UBYTE * buffer_start = buffer;
	ULONG block = 0;
	
	/* obtain display id from display number */
	ULONG display_id = Get_Display_ID(display);
	if (display_id == -1)
		return (0);
	
	/* obtain edid data from display id */
	if (Get_EDID_Block_Display(display_id, block, buffer))
	{
		if (Checksum(buffer, EDID_BLOCK_SIZE) == 0)
		{
			UBYTE extensions = buffer[126];
			
			buffer += EDID_BLOCK_SIZE;
			
			/* obtain edid extensions if any */
			if (extensions > 0 && extensions < EDID_BLOCK_COUNT)
			{
				for (block = 1; block <= extensions; block++)
				{
					if (Get_EDID_Block_Display(display_id, block, buffer))
					{
						buffer += EDID_BLOCK_SIZE;
					}
					else
					{
						break;
					}
				}
			}
		}
		else
		{
			PutStr("Bad checksum\n");
		}
	}
	
	/* return edid size */
	return (ULONG)(buffer - buffer_start);
}

/******************************************************************************
 * 
 * End of file
 * 
 *****************************************************************************/
