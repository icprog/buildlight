// USBCMDAP.cpp : Defines the entry point for the console application.
//



#include "stdafx.h"		
#include "windows.h"		
#include <stdio.h>		// printf()
#include <stdlib.h>		// exit()
#include <string.h>		// memset()
#include <conio.h>		// _kbhit()
#include <ctype.h>		// toupper()
#include "USBHIDIO.h"	// Delcom USB functions


//#include <objbase.h>
//#include <initguid.h>



 

const TCHAR Menu[] = 
L"USBCMDAP - USB HID I/O Command Application - Delcom Products Inc.\r\n"          
L"This utility allows Delcom USB HID commands to be sent to the USB device.\r\n"
L"Refer to the Delcom USBIOHID Datasheet for commands.\r\n"
L"www.delcomproducts.com/downloads/USBIOHID.pdf\r\n"
L"Syntax: USBCMDAP [options] TID SID MajorCmd MinorCmd \r\n"
L"                 [DataLSB DataMSB DataHID0..3 DataEXT0..7] \r\n"
L"Options: (no spaces between options e.g. \'SWV\')\r\n"
L"E - Enumerate. Displays a list of all the Delcom USB device found.\r\n"
L"S - Shared. Opens USB device as a shared device.\r\n"
L"    Allows other applications to access it at the same time.\r\n"
L"W - Wait. Program will wait for the user to press a key before it exits.\r\n"
L"V - Verbose On. Displays all verbose information.\r\n"
L"TID - TypeID - Search for: 0=All, 1=USBIO, 2=USBVI, 3=USBND, 4=USBTL, 5=USBBUZ \r\n"
L"SID - Serial ID - Search for: 0=All, otherwise device with a matching SID.\r\n"
L"MajorCmd - The major command.\r\n"
L"MinorCmd - The minor command.\r\n"
L"LSBData - The LSB data paramater.\r\n"
L"MSBData - The MSB data parameter.\r\n"
L"DataHID0..3 - The HID data parameters(4) \r\n"
L"DataExt0..7 - The Extension data paramaters(8).\r\n"
L"\r\n"
L"Example: To set pin 0 on port 1 low (P1.0).\r\n"
L"USBCMPAP 0 0 101 12 1 0\r\n";

typedef unsigned char U8;

typedef struct ErrorStruct_ {
	union {
		unsigned int All;
		unsigned int Any;
		struct {
			unsigned int Syntax:1;
			unsigned int DeviceNotFound:1;
			unsigned int FailedtoRead:1;
			unsigned int FailedtoWrite:1;
			
			};
		};
	}ErrorStruct, &pErrorStruct;

typedef struct OptionStruct_ {
	union {
		unsigned int All;
		unsigned int Any;
		struct {
			unsigned int Enumerate:1;
			unsigned int Shared:1;
			unsigned int Wait:1;
			unsigned int Verbose:1;

			};
		};
	}OptionStruct, &pOptionStruct;

typedef struct ParameterStruct_ {
	ErrorStruct Errors;
	OptionStruct Options;
	unsigned int TID;
	unsigned int SID;
	HIDPacketStruct Cmd;
	
	}ParameterStruct, &pParameterStruct;


// Global data variables
ParameterStruct Parameters;		

CUSBHIDIO CUsb;			// The Delocm Device Class (defined in USBHIDIO.h)

// Prototyping
void AllDone();

// ------------------------------------------------------------ //
// Main Entry
// ------------------------------------------------------------ //
int _tmain(int argc, _TCHAR* argv[])
{
		// Variable
		int index,ptr;
		TCHAR *stopstr;
		memset(&Parameters, 0, sizeof(Parameters));		// zero it out
	
	
		index=1;	// point to the first user parameter, zero points to path.
		
		// Parse options first
		if( argc >= 2 ) {	// 
			ptr = 0;
			while(argv[index][ptr] != NULL ) {
				if(toupper(argv[index][ptr]) == 'E') Parameters.Options.Enumerate = true;
				if(toupper(argv[index][ptr]) == 'S') Parameters.Options.Shared = true;
				if(toupper(argv[index][ptr]) == 'V') Parameters.Options.Verbose = true;
				if(toupper(argv[index][ptr]) == 'W') Parameters.Options.Wait = true;
				ptr++;
				}
			if( Parameters.Options.Any) index++;	// only increment if an option is found
		}

		// If enumerate, scan for device and display and exit.
		if(Parameters.Options.Enumerate) {
			_tprintf(L"USB HID I/O Command Application - Searching for all Delcom devices...\r\n");
			CUsb.ScanForHIDDevice(USB_VID, USB_PID, 0xFFFF, 0, 1, Parameters.Options.Shared);
			CUsb.CloseDevice();
			AllDone();
			}
		
		if( argc < 4 ) {	// User must pass at least 3 parameters(TID,SID,Major) -on error print menu
			Parameters.Errors.Syntax = true;
			_tprintf(Menu);
			AllDone();
			}
	


		// ok at this point we chave the require parameters, now get them
		if(index<argc) Parameters.TID = (unsigned int)_tcstol(argv[index++],&stopstr,10 );
		if(index<argc) Parameters.SID = (unsigned int)_tcstol(argv[index++],&stopstr,10);
		if(index<argc) Parameters.Cmd.Tx.MajorCmd = (unsigned char)_tcstol(argv[index++],&stopstr,10);
		if(index<argc) Parameters.Cmd.Tx.MinorCmd = (unsigned char)_tcstol(argv[index++],&stopstr,10);
		if(index<argc) Parameters.Cmd.Tx.DataLSB = (unsigned char)_tcstol(argv[index++],&stopstr,10);
		if(index<argc) Parameters.Cmd.Tx.DataMSB = (unsigned char)_tcstol(argv[index++],&stopstr,10);
		ptr=0;
		while(index<argc) Parameters.Cmd.Tx.DataHID[ptr++] = (unsigned char)_tcstol(argv[index++],&stopstr,10);
		ptr=0;
		while(index<argc) Parameters.Cmd.Tx.DataExt[ptr++] = (unsigned char)_tcstol(argv[index++],&stopstr,10);
		
		


		// If verbose display title
		if(Parameters.Options.Verbose) {
			_tprintf(L"USB HID I/O Command Application v0.1 Options: ");
			if(Parameters.Options.Any){
				if(Parameters.Options.Enumerate) _tprintf(L"E");
				if(Parameters.Options.Shared) _tprintf(L"S");
				if(Parameters.Options.Verbose) _tprintf(L"V");
				if(Parameters.Options.Wait) _tprintf(L"W");
			}
			else _tprintf(L"None");
			_tprintf(L"\r\n");
			_tprintf(L"TID =%u SID=%u ", Parameters.TID, Parameters.SID); 
			_tprintf(L"Maj=%u Min=%u LSB=%u MSB=%u HID0=%u HID1=%u HID2=%u HID3=%u\r\n",
				(U8)Parameters.Cmd.Tx.MajorCmd, (U8)Parameters.Cmd.Tx.MinorCmd, (U8)Parameters.Cmd.Tx.DataLSB, (U8)Parameters.Cmd.Tx.DataMSB, 
				(U8)Parameters.Cmd.Tx.DataHID[0], (U8)Parameters.Cmd.Tx.DataHID[1], (U8)Parameters.Cmd.Tx.DataHID[2], (U8)Parameters.Cmd.Tx.DataHID[3] );
			_tprintf(L"DExt0=%u DExt1=%u DExt2=%u DExt3=%u DExt4=%u DExt5=%u DExt6=%u DExt7=%u\r\n",
				(U8)Parameters.Cmd.Tx.DataExt[0], (U8)Parameters.Cmd.Tx.DataExt[1], (U8)Parameters.Cmd.Tx.DataExt[2], (U8)Parameters.Cmd.Tx.DataExt[3], 
				(U8)Parameters.Cmd.Tx.DataExt[4], (U8)Parameters.Cmd.Tx.DataExt[5], (U8)Parameters.Cmd.Tx.DataExt[6], (U8)Parameters.Cmd.Tx.DataExt[7] );

		}

		// ----------------------------- //
		// open the Device
		// ----------------------------- //
		CUsb.ScanForHIDDevice(USB_VID, USB_PID, Parameters.TID, Parameters.SID,0, Parameters.Options.Shared);
		if(!CUsb.IsOpen()) {	// Delcom USB device not found
			Parameters.Errors.DeviceNotFound = true;
			AllDone();
			}

		else{// Device found
			if(Parameters.Options.Verbose) {
				_tprintf(L"Device Found: %s\r\n",CUsb.DeviceName);
				DelcomDeviceInfoStruct Info;
				if(CUsb.GetDeviceInfo(&Info)) {
					Parameters.Errors.FailedtoRead = true;
					AllDone();
					}
				// Display Device Type, Version & Serial Number
				_tprintf(L"DeviceType:%u  Serial#:%u  Version:%u  DateCode:%u/%u/%u\r\n",
					Info.Family, Info.Serial, Info.Version, Info.Month, Info.Day, 2000+Info.Year);
				}
		
			if( Parameters.Cmd.Tx.MajorCmd == 101 || Parameters.Cmd.Tx.MajorCmd == 102 )  // write command
				{
				if(CUsb.WriteCmd((pHIDPacketStruct)&Parameters.Cmd.Tx,16)) Parameters.Errors.FailedtoWrite = true;
				}

			else{
				if(CUsb.ReadCmd((pHIDPacketStruct)&Parameters.Cmd.Rx,16)) Parameters.Errors.FailedtoRead = true;
				if(Parameters.Options.Verbose) {	// print out read data
					_tprintf(	L"Read command data result:\r\n");
					_tprintf(	L"B0=%u B1=%u B2=%u B3=%u B4=%u B5=%u B6=%u B7=%u \r\n"
								L"B8=%u B9=%u B10=%u B11=%u B12=%u B13=%u B14=%u B15=%u \r\n",
								(U8)Parameters.Cmd.Data[0], (U8)Parameters.Cmd.Data[1], (U8)Parameters.Cmd.Data[2], (U8)Parameters.Cmd.Data[3], 
								(U8)Parameters.Cmd.Data[4], (U8)Parameters.Cmd.Data[5], (U8)Parameters.Cmd.Data[6], (U8)Parameters.Cmd.Data[7], 
								(U8)Parameters.Cmd.Data[8], (U8)Parameters.Cmd.Data[9], (U8)Parameters.Cmd.Data[10], (U8)Parameters.Cmd.Data[11], 
								(U8)Parameters.Cmd.Data[12], (U8)Parameters.Cmd.Data[13], (U8)Parameters.Cmd.Data[14], (U8)Parameters.Cmd.Data[15]);
						}
				}

			}


		CUsb.CloseDevice();			// always close the device when done
		AllDone();
		
			

	return 0;
}


void AllDone()
{

	if(Parameters.Errors.Any) {
		_tprintf(L"Error:(0x%X) ", Parameters.Errors.All);
		if(Parameters.Errors.DeviceNotFound) _tprintf(L"Device not found!");
		if(Parameters.Errors.Syntax) _tprintf(L"Syntax error!");
		if(Parameters.Errors.FailedtoRead) _tprintf(L"Failed to read device!");
		if(Parameters.Errors.FailedtoWrite) _tprintf(L"Failed to write device!");
		_tprintf(L"\r\n");
		}
	else {
		 if(Parameters.Options.Verbose) _tprintf(L"Command completed.\r\n"); 
		}
	if(Parameters.Options.Wait) { 	_tprintf(L"Press any key to continue..."); while(!_kbhit());	} // wait 

	exit(0);
}
