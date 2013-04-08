// USBHIDIO.cpp : code file
// Delcom Products Inc.
// USB HID Functions

#pragma once
#include "stdafx.h"
#include "windows.h"	
#include "stdlib.h"
#include "ctype.h"
#include "USBHIDIO.h"

// NOTE - The following lib files must be added to the project 
// in Linker->Input->additional_dependencies and phyiscally to the project
// setupapi.lib hid.lib
// Also the following header file must be added to the project
// hidsdi.h hidpi.h hidusage.h


// Required functions for HID API
extern "C" {		// note this exten "C" is required 
#include "hidsdi.h"
#include "setupapi.h"
}


// CUSBHIDIO Constuctor
CUSBHIDIO::CUSBHIDIO()
{
	hDevice = 0;

}



//MessageBox(NULL, "hello world", "hw", MB_OK);




// ------------------------------------------------------ //
// Closes the device
// ------------------------------------------------------ //
unsigned int CUSBHIDIO::CloseDevice(void)
{
	//KillTimer(ID_TIMER);
	if(hDevice) {
		CloseHandle(hDevice);
		hDevice = 0;
		}
	return 0;
}

// ------------------------------------------------------ //
// Find and open the USB HID IO Device
// ------------------------------------------------------ //
unsigned int CUSBHIDIO::OpenDevice(void)
{
	return(ScanForHIDDevice(USB_VID, USB_PID, USB_TID, USB_SID, 0, 0));
	
}




// ------------------------------------------------------ //
// ScanForHidDevice(VID,PID,TID) - Scan thru all the HID device lookking
// for a match on the VID, PID and optional TID (Type ID) and SID (Serial Number).
// Sets the hDevice handle varible if found and opens the device
// Return zero if found, else non-zero error code.
// 0 = Success
// 1 = No matching HID devices
// Enum if set - prints all the devices found
// ------------------------------------------------------ //
unsigned int CUSBHIDIO::ScanForHIDDevice(unsigned int VID, unsigned int PID, unsigned int TID, unsigned int SID, unsigned int Enum, unsigned int ShareMode )
{
	//Use a series of API calls to find a HID with a matching Vendor,Product, Type and Serial ID.
	DelcomDeviceInfoStruct		DelcomInfo;
	PSP_DEVICE_INTERFACE_DETAIL_DATA	detailData;
	GUID						HidGuid;
	HANDLE						hDevInfo;
	ULONG						Required;
	HIDD_ATTRIBUTES				Attributes;
	SP_DEVICE_INTERFACE_DATA	devInfoData;
	bool						LastDevice = FALSE;
	int							MemberIndex = 0;
	bool						MyDeviceDetected = FALSE; 
	LONG						Result;
	ULONG						Length;
	ULONG						devcnt;
	DWORD						SHARE_MODE;
	
	// Variable init	
	devcnt=0;
	Length = 0;
	detailData = NULL;
	hDevice=NULL;
	MemberIndex = 0;
	LastDevice = FALSE;

	if(ShareMode)	SHARE_MODE = FILE_SHARE_READ|FILE_SHARE_WRITE;
	else			SHARE_MODE = 0;

	
	// API function: HidD_GetHidGuid - Get the GUID for all system HIDs. Returns: the GUID in HidGuid.
	HidD_GetHidGuid(&HidGuid);	
	
	// API function: SetupDiGetClassDevs -Returns: a handle to a device information set for all installed devices.
	// Requires: the GUID returned by GetHidGuid.
	hDevInfo=SetupDiGetClassDevs(&HidGuid, NULL, NULL, DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);		
	devInfoData.cbSize = sizeof(devInfoData);

	//Step through the available devices looking for the one we want. 
	//Quit on detecting the desired device or checking all available devices without success.
	
	do	{
		MyDeviceDetected=FALSE;
		
		//API function: SetupDiEnumDeviceInterfaces - On return, MyDeviceInterfaceData contains the handle to a
		// SP_DEVICE_INTERFACE_DATA structure for a detected device. Requires:
		// The DeviceInfoSet returned in SetupDiGetClassDevs. The HidGuid returned in GetHidGuid. An index to specify a device.
		
		Result=SetupDiEnumDeviceInterfaces(hDevInfo, 0, &HidGuid, MemberIndex, &devInfoData);
		if (Result != 0)
			{	//A device has been detected, so get more information about it.

			// API function: SetupDiGetDeviceInterfaceDetail - Returns: an SP_DEVICE_INTERFACE_DETAIL_DATA structure
			// containing information about a device. To retrieve the information, call this function twice.
			// The first time returns the size of the structure in Length. The second time returns a pointer to the data in DeviceInfoSet.
			// Requires: A DeviceInfoSet returned by SetupDiGetClassDevs The SP_DEVICE_INTERFACE_DATA structure returned by SetupDiEnumDeviceInterfaces.
			// The final parameter is an optional pointer to an SP_DEV_INFO_DATA structure. This application doesn't retrieve or use the structure.			
			// If retrieving the structure, set MyDeviceInfoData.cbSize = length of MyDeviceInfoData.
			// and pass the structure's address.
						
			//Get the Length value.
			//The call will return with a "buffer too small" error which can be ignored.
			Result = SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, NULL, 0, &Length, NULL);

			//Allocate memory for the hDevInfo structure, using the returned Length.
			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);
			
			//Set cbSize in the detailData structure.
			detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			//Call the function again, this time passing it the returned buffer size.
			Result = SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, detailData, Length, &Required, NULL);

			//Open a handle to the device.
			// API function: CreateFile - Returns: a handle that enables reading and writing to the device.
			// Requires: The DevicePath in the detailData structure	returned by SetupDiGetDeviceInterfaceDetail.
			// NOTES: details->DevicePath - holds the name of the device - 
			// NOTES: For file sharing set 3rd parameter to FILE_SHARE_READ|FILE_SHARE_WRITE
			hDevice=CreateFile(detailData->DevicePath, GENERIC_READ|GENERIC_WRITE, SHARE_MODE,
								(LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);

			// API function: HidD_GetAttributes - Requests information from the device.
			// Requires: the handle returned by CreateFile.	Returns: a HIDD_ATTRIBUTES structure containing
			// the Vendor ID, Product ID, and Product Version Number. Use this information to decide if the detected device is
			// the one we're looking for.
			
			//Set the Size to the number of bytes in the structure.
			Attributes.Size = sizeof(Attributes);
			Result = HidD_GetAttributes(hDevice, &Attributes);
						
			//Is it the desired device?
			MyDeviceDetected = FALSE;
			if( (Attributes.VendorID == VID) && (Attributes.ProductID == PID))
				{	//Both the Product and Vendor IDs match. Now check for TID and SID if none zero
				MyDeviceDetected = TRUE;   // premark it found
				// To enable overlappedtransfer use: GetDeviceCapabilities() & PrepareForOverlappedTransfer()
				_tcsncpy_s(DeviceName, sizeof(DeviceName), detailData->DevicePath, 512);		// save the devicename
				if(TID || SID || Enum) {	// Now check for TID and SID if non-zero
					if(GetDeviceInfo(&DelcomInfo)) MyDeviceDetected = FALSE;		// this function must succeed
					else {
						if(TID && (DelcomInfo.Family != TID)) MyDeviceDetected = FALSE;
						if(SID && (DelcomInfo.Serial != SID)) MyDeviceDetected = FALSE;
						if(Enum) {
							_tprintf(L"Device Found(%u): TID=%u SID=%u.\r\n%s\r\n", ++devcnt, DelcomInfo.Family,DelcomInfo.Serial, DeviceName);
							MyDeviceDetected = FALSE;		// enum option does open the file
							}
						}
					}	// end of TID or SID
				
				} 

			else	{	//The PID and/or VID doesn't match. Close the device try the next one
				CloseHandle(hDevice); 
				hDevice = 0;
				} 
			

			//Free the memory used by the detailData structure (no longer needed).
			free(detailData);
		}  //if (Result != 0)

		else	{				// End of List - No HID devices detected!
			LastDevice=TRUE;	//SetupDiEnumDeviceInterfaces returned 0, so there are no more devices to check.
			}

		//If we haven't found the device yet, and haven't tried every available device,
		//try the next one.
		MemberIndex++;

	} // loop till either end of deivce list or we find our device 
	while ((LastDevice == FALSE) && (MyDeviceDetected == FALSE));


	SetupDiDestroyDeviceInfoList(hDevInfo);		//Free the memory reserved for hDevInfo by SetupDiClassDevs.

	if (MyDeviceDetected == FALSE) {
		// Device not found
		hDevice = 0;
		return(1);
		}
	else	{
		// Device Found
		HidD_SetNumInputBuffers(hDevice,1);					//sets the maximum number of input reports that the HID class driver ring buffer can hold for a specified top-level collection.
		return(0); // Success
		}

	

	

	
}

// ---------------------------------------------------------------- //
// Return ture if the device is currently openned
// ---------------------------------------------------------------- //
int CUSBHIDIO::IsOpen(void)
{	
	if(hDevice) return(1);
	else		return(0);
}


// ---------------------------------------------------------------- //
// Writes a command to the USB HID Device
// ---------------------------------------------------------------- //
int CUSBHIDIO::WriteCmd(pHIDPacketStruct pPacket, unsigned int Length)
{
	if(!hDevice) return(1);

	if(HidD_SetFeature(hDevice,pPacket, Length)) {

		return(0);		// success
		}

	return(1);
}

// ---------------------------------------------------------------- //
// Read a command to the USB HID Device
// ---------------------------------------------------------------- //
int CUSBHIDIO::ReadCmd(pHIDPacketStruct pPacket, unsigned int Length)
{
	if(!hDevice) return(1);

	if(HidD_GetFeature(hDevice,pPacket, Length)) {

		return(0);		// success
		}

	return(1);
}


// ---------------------------------------------------------------- //
// Reads device info
// Retruns zero on success, else non-zero
// Return data in a 16byte data buffer. Buffer must be predeclared by called
// ---------------------------------------------------------------- //
int CUSBHIDIO::GetDeviceInfo(pDelcomDeviceInfoStruct pInfo)
{
	myPacket.Rx.Cmd = 104;

	if(!HidD_GetFeature(hDevice,&myPacket, 16)) {
		
		return(1);	// command failed
		}

	// now get the dat if the variable has been passed
	if(!pInfo) return(1); 
	memcpy(pInfo,&myPacket,16);
	return 0;
}


// ---------------------------------------------------------------- //
// Reads the I/O port pin values,
// BYTE0 = PORT0, BYTE1 = PORT1
// Retruns zero on success, else non-zero 
// ---------------------------------------------------------------- //
int CUSBHIDIO::ReadPorts(unsigned int *pPorts)
{
	myPacket.Rx.Cmd = 100;

	if(!HidD_GetFeature(hDevice,&myPacket, 8)) {
		
		return(1);	// command failed
		}
	*pPorts = *(unsigned int*)&myPacket.Data[0];
	
	return 0;
}


