/******************************************************************************
 * 
 * Program:  Emu68EDID.c
 * Purpose:  PiStorm/Emu68 utility to dump/decode EDID data.
 * Target:   M68K AmigaOS 3.x, PiStorm/Emu68 1.1+
 * Compiler: SAS/C Amiga Compiler 6.59
 * Author:   Philippe CARPENTIER
 * 
 * Usage:
 * Emu68EDID DUMP [DISPLAY=<num>] [TO=<file>]
 * Emu68EDID PARSE [DISPLAY=<num>] [FROM=<file>] [FULL]
 * 
 * Examples:
 * Emu68EDID DUMP                ; HEX output of the primary display.
 * Emu68EDID DUMP >EDID.txt      ; HEX output of the primary display in file.
 * Emu68EDID DUMP TO=EDID.raw    ; RAW output of the primary display in file.
 * Emu68EDID DUMP DISPLAY=0      ; HEX output of the primary display.
 * Emu68EDID DUMP DISPLAY=1      ; HEX output of the secondary display.
 * Emu68EDID PARSE               ; Parse from the attached primary display.
 * Emu68EDID PARSE FULL          ; Parse from the attached primary display, detailed.
 * Emu68EDID PARSE FROM=EDID.raw ; Parse from the provided EDID raw file.
 * Emu68EDID PARSE DISPLAY=0     ; Parse from the attached primary display.
 * Emu68EDID PARSE DISPLAY=1     ; Parse from the attached secondary display.
 * 
 ******************************************************************************/

#include <dos/dos.h>
#include <exec/exec.h>
#include <resources/mailbox.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/mailbox.h>
#include <proto/utility.h>

#include "Main.h"
#include "DumpEDID.h"
#include "DecodeEDID.h"

/******************************************************************************
 * 
 * DEFINES
 * 
 ******************************************************************************/

#define TEMPLATE "DISPLAY/N,DUMP/S,TO/K,PARSE/S,FROM/K,FULL/S"

typedef enum {
	OPT_DISPLAY,
	OPT_DUMP, OPT_TO,
	OPT_PARSE, OPT_FROM, OPT_FULL,
	OPT_COUNT
} OPT_ARGS;

/******************************************************************************
 * 
 * GLOBALS
 * 
 ******************************************************************************/

APTR MailboxBase = NULL;
STATIC UBYTE * EDIDData = NULL;
CONST_STRPTR verstring = VERSTRING;

/******************************************************************************
 * 
 * EXTERNS
 * 
 ******************************************************************************/

extern struct ExecBase * SysBase;
extern struct DosLibrary * DOSBase;
extern struct Library * UtilityBase;

/******************************************************************************
 * 
 * PROTOTYPES
 * 
 ******************************************************************************/

STATIC LONG EDID_DumpToFile(STRPTR filename, UBYTE * buffer, ULONG size);
STATIC VOID EDID_DumpToStdout(UBYTE * buffer, ULONG size);
STATIC LONG EDID_Dump(LONG display, STRPTR filename);

STATIC LONG EDID_ParseFromFile(STRPTR filename, BOOL full);
STATIC LONG EDID_ParseFromHardware(LONG display, BOOL full);
STATIC LONG EDID_Parse(LONG display, STRPTR filename, BOOL full);

/******************************************************************************
 * 
 * EDID_DumpToFile()
 * 
 ******************************************************************************/

STATIC LONG EDID_DumpToFile(STRPTR filename, UBYTE * buffer, ULONG size)
{
	STATIC BPTR file;
	STATIC LONG errorCode = 0;
	
	if (file = Open(filename, MODE_NEWFILE))
	{
		if (Write(file, buffer, size) != size)
		{
			errorCode = IoErr();
		}
		
		Close(file);
	}
	
	return (errorCode);
}

/******************************************************************************
 * 
 * EDID_DumpToStdout()
 * 
 ******************************************************************************/

STATIC VOID EDID_DumpToStdout(UBYTE * buffer, ULONG size)
{
	STATIC ULONG i;
	
	for (i = 0; i < size; i++)
	{
		Printf("%02lx ", buffer[i]);
		
		if ((i + 1) % 16 == 0)
		{
			PutStr("\n");
		}
	}
}

/******************************************************************************
 * 
 * EDID_Dump()
 * 
 ******************************************************************************/

STATIC LONG EDID_Dump(LONG display, STRPTR filename)
{
	ULONG edid_size = 0;
	
	if (display >= 0)
	{
		edid_size = Get_EDID_Display(display, EDIDData);
	}
	else
	{
		edid_size = Get_EDID_Primary(EDIDData);
	}
	
	if (edid_size > 0)
	{
		if (filename != NULL)
		{
			return EDID_DumpToFile(filename, EDIDData, edid_size);
		}
		
		EDID_DumpToStdout(EDIDData, edid_size);
	}
	else
	{
		SetIoErr(ERROR_NO_MORE_ENTRIES);
	}
	
	return (IoErr());
}

/******************************************************************************
 * 
 * EDID_ParseFromFile()
 * 
 ******************************************************************************/

STATIC LONG EDID_ParseFromFile(STRPTR filename, BOOL full)
{
	BPTR file;
	APTR buffer;
	MonitorInfo * info;
	struct FileInfoBlock __aligned fib;
	
	if (file = Open(filename, MODE_OLDFILE))
	{
		if (ExamineFH(file, &fib))
		{
			if (buffer = AllocVec(fib.fib_Size + 1, MEMF_PUBLIC | MEMF_CLEAR))
			{
				if (Read(file, buffer, fib.fib_Size) == fib.fib_Size)
				{
					if (info = decode_edid(buffer))
					{
						if (full) dump_monitor_info(info);
						else dump_monitor_info_short(info);
						free_monitor_info(info);
					}
					else
					{
						PutStr("parse error\n");
					}
				}
				
				FreeVec(buffer);
			}
			else
			{
				SetIoErr(ERROR_NO_FREE_STORE);
			}
		}
		
		Close(file);
	}
	
	return (IoErr());
}

/******************************************************************************
 * 
 * EDID_ParseFromHardware()
 * 
 ******************************************************************************/

STATIC LONG EDID_ParseFromHardware(LONG display, BOOL full)
{
	ULONG edid_size = 0 ;
	
	if (display >= 0)
	{
		edid_size = Get_EDID_Display(display, EDIDData);
	}
	else
	{
		edid_size = Get_EDID_Primary(EDIDData);
	}
	
	if (edid_size > 0)
	{
		MonitorInfo * info;
		
		if (info = decode_edid(EDIDData))
		{
			if (full) dump_monitor_info(info);
			else dump_monitor_info_short(info);
			free_monitor_info(info);
		}
	}
	else
	{
		SetIoErr(ERROR_NO_MORE_ENTRIES);
	}
	
	return (IoErr());
}

/******************************************************************************
 * 
 * EDID_Parse()
 * 
 ******************************************************************************/

STATIC LONG EDID_Parse(LONG display, STRPTR filename, BOOL full)
{
	if (filename != NULL)
	{
		return EDID_ParseFromFile(filename, full);
	}
	
	return EDID_ParseFromHardware(display, full);
}

/******************************************************************************
 * 
 * InitLibs()
 * 
 ******************************************************************************/

STATIC BOOL InitLibs(VOID)
{
	if ((MailboxBase = OpenResource(MAILBOXNAME)) == NULL)
	{
		PutStr(MAILBOXNAME " not found\n");
		return FALSE;
	}
	
	if (!(EDIDData = AllocVec(
		EDID_BLOCK_SIZE * EDID_BLOCK_COUNT, 
		MEMF_PUBLIC | MEMF_CLEAR)))
	{
		PutStr("memory allocation error\n");
		return FALSE;
	}
	
	return TRUE;
}

/******************************************************************************
 *
 * CleanExit()
 *
 ******************************************************************************/

STATIC VOID CleanExit(VOID)
{
	if (EDIDData != NULL)
	{
		FreeVec(EDIDData);
		EDIDData = NULL;
	}
}

/******************************************************************************
 * 
 * main()
 * 
 ******************************************************************************/

ULONG main(ULONG argc, STRPTR * argv)
{
	ULONG rc = RETURN_FAIL;
	LONG errorCode = 0;
	LONG opts[OPT_COUNT];
	struct RDArgs * rdargs;
	
	if (!InitLibs())
	{
		CleanExit();
		return (RETURN_FAIL);
	}
	
	opts[OPT_DUMP   ] =  0;
	opts[OPT_DISPLAY] = -1;
	opts[OPT_TO     ] =  0;
	opts[OPT_PARSE  ] =  0;
	opts[OPT_FROM   ] =  0;
	opts[OPT_FULL   ] =  0;
	
	if (rdargs = (struct RDArgs *)ReadArgs(TEMPLATE, opts, NULL))
	{
		rc = RETURN_ERROR;
		
		if (opts[OPT_DUMP])
		{
			errorCode = EDID_Dump(
				*(LONG *)opts[OPT_DISPLAY], 
				 (STRPTR)opts[OPT_TO]);
			
			rc = (errorCode == 0) ? RETURN_OK : RETURN_WARN;
		}
		else if (opts[OPT_PARSE])
		{
			errorCode = EDID_Parse(
				*(LONG *)opts[OPT_DISPLAY], 
				 (STRPTR)opts[OPT_FROM], 
				         opts[OPT_FULL] ? TRUE : FALSE);
			
			rc = (errorCode == 0) ? RETURN_OK : RETURN_WARN;
		}
		else
		{
			errorCode = ERROR_REQUIRED_ARG_MISSING;
		}
		
		FreeArgs(rdargs);
	}
	else
	{
		errorCode = IoErr();
	}
	
	if (errorCode)
	{
		PrintFault(errorCode, NULL);
	}
	
	CleanExit();
	
	return (rc);
}

/******************************************************************************
 * 
 * End of file
 * 
 ******************************************************************************/
