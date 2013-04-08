// USBHIDIO.h : header file
// Delcom Products Inc.
// USB HID Functions

#pragma once


// The following constants determine which HID device to find and connect to.
#define USB_VID 0x0FC5		// USB Vendor ID (Always 0x0FC5 for Delcom products)
#define USB_PID 0xB080		// USB Product ID (Always 0xB080 for Delcom HID device)
#define USB_TID 0x0000		// USB Type ID	(1 for Delcom USBHIDIO device, zero=scan for all)
#define USB_SID	0x0000		// USB Serial ID (zero=scan for all)

enum FamilyType{
	ALL,					// all Delcom USB device
	USBIO,					// all Delcom USB IO Chips and foot switch
	USBVI,					// all Delcom USB Visual Indicators
	USBND,					// all Delcom USB Numeric Displays
	USBBUZ					// all Delcom USB Buzzers
};


// Packet DataStruct
typedef union HIDPacketStruct { 
	char Data[16];
	struct {
		char MajorCmd;
		char MinorCmd;
		char DataLSB;
		char DataMSB;
		char DataHID[4];
		char DataExt[8];
		} Tx;
	struct {
		char Cmd;
		} Rx;

}	HIDPacketStruct, *pHIDPacketStruct; 

// DataStruct used by the GetDeviceInfo functions
typedef struct DelcomDeviceInfoStruct_ {
		unsigned short int Family;
		unsigned short int Security;
		unsigned char Version;
		unsigned char Day;
		unsigned char Month;
		unsigned char Year;
		unsigned int Serial;
		unsigned int Spare;
		} DelcomDeviceInfoStruct, *pDelcomDeviceInfoStruct;




// THE USBHIDIO class
class CUSBHIDIO
{
	protected:
		HANDLE hDevice; // Handle to the device, zero if not openned
		HIDPacketStruct myPacket;

	public:
		TCHAR DeviceName[512];
		CUSBHIDIO();			// Construction
		unsigned int CloseDevice(void);
		unsigned int OpenDevice(void);
		unsigned int ScanForHIDDevice(unsigned int VID, unsigned int PID, unsigned int TID, unsigned int SID, unsigned int Enum, unsigned int Share  );
		



public:
	// // Return ture if the device is currently openned
	int IsOpen(void);
	int WriteCmd(pHIDPacketStruct pPacket, unsigned int Length);
	int ReadCmd(pHIDPacketStruct pPacket, unsigned int Length);
	
	int CUSBHIDIO::GetDeviceInfo(pDelcomDeviceInfoStruct pInfo);
	int ReadPorts(unsigned int *pPorts);
};


