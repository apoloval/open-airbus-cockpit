/*
 * This file is part of Open Airbus Cockpit
 * Copyright (C) 2002-2012 Peter Dowson, Pelle Liljendal and Chris Brett
 *
 * Open Airbus Cockpit is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Open Airbus Cockpit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open Airbus Cockpit. If not, see <http://www.gnu.org/licenses/>.
 */

/* MODULEUSER.C	Internal User interface library for FSUIPC
*******************************************************************************

Started:          11th July 2000

With acknowledgements to Adam Szofran (author of original FS6IPC).

******************************************************************************/

#define WM_IPCTHREADACCESS (WM_USER+130)
#define LIB_VERSION 2001 // 2.001
#define MAX_SIZE 0x7F00 // Largest data (kept below 32k to avoid
								// any possible 16-bit sign problems)
#ifdef UNICODE
#define UIPC_MAIN L"UIPCMAIN"
#else
#define UIPC_MAIN "UIPCMAIN"
#endif

#include "IPCuser.h"

/******************************************************************************
			IPC client stuff
******************************************************************************/

DWORD FSUIPC_Version = 0;
DWORD FSUIPC_FS_Version = 0;
DWORD FSUIPC_Lib_Version = LIB_VERSION;

static HWND    m_hWnd = 0;       // FS6 window handle
static BYTE*   m_pView = 0;      // pointer to view of data object
static BYTE*   m_pNext = 0;
static unsigned long m_ulMax = 0;

/******************************************************************************
			FSUIPC_Close
******************************************************************************/

// Stop the client
void FSUIPC_Close(void)
{	m_hWnd = 0;
	m_pView = 0;
}

/******************************************************************************
			FSUIPC_Open
******************************************************************************/

// Start the client
// return: TRUE if successful, FALSE otherwise
BOOL FSUIPC_Open2(DWORD dwFSReq, DWORD *pdwResult, BYTE *pMem, DWORD dwSize)
{  // abort if already started
	if (m_pView)
	{	*pdwResult = FSUIPC_ERR_OPEN;
		return FALSE;
	}

	if (!pMem || (dwSize < 32))
	{	*pdwResult = FSUIPC_ERR_VIEW;
		return FALSE;
	}

	// Clear version information, so know when connected
	FSUIPC_Version = FSUIPC_FS_Version = 0;
	
	// Connect via FSUIPC, which is known to be FSUIPC's own
	// and isn't subject to user modificiation
   m_hWnd = FindWindowEx(NULL, NULL, UIPC_MAIN, NULL);
	if (!m_hWnd)
	{	*pdwResult = FSUIPC_ERR_NOFS;
		return FALSE;
	}
	
	// get an area of memory
	m_ulMax = min(dwSize, MAX_SIZE);
	m_pView = pMem;
	
	// Okay, now determine FSUIPC version AND FS type
	m_pNext = m_pView + 4; // Allow space for code pointer

	// Read FSUIPC version
	if (!FSUIPC_Read(0x3304, 4, &FSUIPC_Version, pdwResult))
	{	FSUIPC_Close();
		return FALSE;
	}

	// and FS version and validity check pattern
	if (!FSUIPC_Read(0x3308, 4, &FSUIPC_FS_Version, pdwResult))
	{	FSUIPC_Close();
		return FALSE;
	}

	// Write our Library version number to a special read-only offset
	// This is to assist diagnosis from FSUIPC logging
	if (!FSUIPC_Write(0x330a, 2, &FSUIPC_Lib_Version, pdwResult))
	{	FSUIPC_Close();
		return FALSE;
	}

	// Actually send the requests and get the responses ("process")
	if (!FSUIPC_Process(pdwResult))
	{	FSUIPC_Close();
		return FALSE;
	}

	// Only allow running on FSUIPC 1.998e or later
	// with correct check pattern 0xFADE
	if ((FSUIPC_Version < 0x19980005) || ((FSUIPC_FS_Version & 0xFFFF0000L) != 0xFADE0000))
	{	*pdwResult = FSUIPC_ERR_VERSION;
		FSUIPC_Close();
		return FALSE;
	}

	FSUIPC_FS_Version &= 0xffff; // Isolates the FS version number
	if (dwFSReq && (dwFSReq != FSUIPC_FS_Version)) // Optional user specific FS request
	{	*pdwResult = FSUIPC_ERR_WRONGFS;
		FSUIPC_Close();
		return FALSE;
	}

	*pdwResult = FSUIPC_ERR_OK;
	return TRUE;
}

/******************************************************************************
			FSUIPC_Process
******************************************************************************/

BOOL FSUIPC_Process(DWORD *pdwResult)
{	DWORD dwError;
	DWORD *pdw;
	FS6IPC_READSTATEDATA_HDR *pHdrR;
	FS6IPC_WRITESTATEDATA_HDR *pHdrW;
	
	if (!m_pView)
	{	*pdwResult = FSUIPC_ERR_NOTOPEN;
		return FALSE;
	}

	if ((m_pView + 4) >= m_pNext)
	{	*pdwResult = FSUIPC_ERR_NODATA;
		return FALSE;
	}

	ZeroMemory(m_pNext, 4); // Terminator
	
	// send the request
	__asm
	{		push eax
			call next
		next: pop eax
			mov dwError,eax
			pop eax
	}
	*((DWORD *) m_pView) = dwError;
	
	dwError = SendMessage(m_hWnd, WM_IPCTHREADACCESS, (WPARAM) (m_pNext - m_pView - 4), (LPARAM) m_pView);

	m_pNext = m_pView + 4;
	
	if (dwError != FS6IPC_MESSAGE_SUCCESS)
	{	*pdwResult = FSUIPC_ERR_DATA; // FSUIPC didn't like something in the data!
		return FALSE;
	}

	// Decode and store results of Read requests
	pdw = (DWORD *) (m_pView + 4);

	while (*pdw)
	{	switch (*pdw)
		{	case FS6IPC_READSTATEDATA_ID:
				pHdrR = (FS6IPC_READSTATEDATA_HDR *) pdw;
				m_pNext += sizeof(FS6IPC_READSTATEDATA_HDR);
				if (pHdrR->pDest && pHdrR->nBytes)
					CopyMemory(pHdrR->pDest, m_pNext, pHdrR->nBytes);
				m_pNext += pHdrR->nBytes;
				break;

			case FS6IPC_WRITESTATEDATA_ID:
				// This is a write, so there's no returned data to store
				pHdrW = (FS6IPC_WRITESTATEDATA_HDR *) pdw;
				m_pNext += sizeof(FS6IPC_WRITESTATEDATA_HDR) + pHdrW->nBytes;
				break;

			default:
				// Error! So terminate the scan
				*pdw = 0;
				break;
		}

		pdw = (DWORD *) m_pNext;
	}

	m_pNext = m_pView + 4;
	*pdwResult = FSUIPC_ERR_OK;
	return TRUE;
}

/******************************************************************************
			FSUIPC_ReadCommon
******************************************************************************/

BOOL FSUIPC_ReadCommon(BOOL fSpecial, DWORD dwOffset, DWORD dwSize, void *pDest, DWORD *pdwResult)
{	FS6IPC_READSTATEDATA_HDR *pHdr = (FS6IPC_READSTATEDATA_HDR *) m_pNext;

	// Check link is open
	if (!m_pView)
	{	*pdwResult = FSUIPC_ERR_NOTOPEN;
		return FALSE;
	}

	// Check have space for this request (including terminator)
	if (((m_pNext - m_pView) + 8 + (dwSize + sizeof(FS6IPC_READSTATEDATA_HDR))) > m_ulMax)
	{	*pdwResult = FSUIPC_ERR_SIZE;
		return FALSE;
	}

	// Initialise header for read request
	pHdr->dwId = FS6IPC_READSTATEDATA_ID;
	pHdr->dwOffset = dwOffset;
	pHdr->nBytes = dwSize;
	pHdr->pDest = (BYTE *) pDest;

	// Initialise the reception area, so rubbish won't be returned
	if (dwSize)
	{	if (fSpecial) CopyMemory(&m_pNext[sizeof(FS6IPC_READSTATEDATA_HDR)], pDest, dwSize);
		else ZeroMemory(&m_pNext[sizeof(FS6IPC_READSTATEDATA_HDR)], dwSize);
	}

	// Update the pointer ready for more data
	m_pNext += sizeof(FS6IPC_READSTATEDATA_HDR) + dwSize;

	*pdwResult = FSUIPC_ERR_OK;
	return TRUE;
}

/******************************************************************************
			FSUIPC_Read
******************************************************************************/

BOOL FSUIPC_Read(DWORD dwOffset, DWORD dwSize, void *pDest, DWORD *pdwResult)
{	return FSUIPC_ReadCommon(FALSE, dwOffset, dwSize, pDest, pdwResult);
}

/******************************************************************************
			FSUIPC_ReadSpecial
******************************************************************************/

BOOL FSUIPC_ReadSpecial(DWORD dwOffset, DWORD dwSize, void *pDest, DWORD *pdwResult)
{	return FSUIPC_ReadCommon(TRUE, dwOffset, dwSize, pDest, pdwResult);
}

/******************************************************************************
			FSUIPC_Write
******************************************************************************/

BOOL FSUIPC_Write(DWORD dwOffset, DWORD dwSize, void *pSrce, DWORD *pdwResult)
{	FS6IPC_WRITESTATEDATA_HDR *pHdr = (FS6IPC_WRITESTATEDATA_HDR *) m_pNext;

	// check link is open
	if (!m_pView)
	{	*pdwResult = FSUIPC_ERR_NOTOPEN;
		return FALSE;
	}
	
	// Check have spce for this request (including terminator)
	if (((m_pNext - m_pView) + 8 + (dwSize + sizeof(FS6IPC_WRITESTATEDATA_HDR))) > m_ulMax)
	{	*pdwResult = FSUIPC_ERR_SIZE;
		return FALSE;
	}

	// Initialise header for write request
	pHdr->dwId = FS6IPC_WRITESTATEDATA_ID;
	pHdr->dwOffset = dwOffset;
	pHdr->nBytes = dwSize;

	// Copy in the data to be written
	if (dwSize) CopyMemory(&m_pNext[sizeof(FS6IPC_WRITESTATEDATA_HDR)], pSrce, dwSize);

	// Update the pointer ready for more data
	m_pNext += sizeof(FS6IPC_WRITESTATEDATA_HDR) + dwSize;

	*pdwResult = FSUIPC_ERR_OK;
	return TRUE;
}

/******************************************************************************
 End of ModuleUser module
******************************************************************************/
