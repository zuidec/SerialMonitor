///
///	SerialCOM is general class used to communicated with COM ports using windows handles
///
#ifndef SERIALCOM_H
#define SERIALCOM_H

#include <string>
#include <windows.h>
#include <vector>

class SerialCOM
{
	public:
		SerialCOM();
		~SerialCOM();


		
	private:
		DWORD COMError;										// Variable to store the results of the windows GetLastError() as needed
		DWORD bytesRead;									// ReadFile requires a pointer to a DWORD to output bytes read. This will be set back to 0 if EOF is reached 
		DWORD bytesWritten;									// WriteFile requires this to output bytes written
		DWORD bytesToWrite;									// Tells WriteFile() how many bytes to write
		HANDLE SerialPort;									// The handle used to access the serial port
		DCB Port;											// Holds parameters used to set up serial port handle
		OVERLAPPED PortOverlap;								// Needed to use asynchronously with ReadFile, not currently used
		COMMTIMEOUTS PortTimeouts;							// Holds all the timeout information for the handle
		std::string LastError;								// Class specific variable to store class defined errors
		std::string CommDCBParameter;						// Contains required data to set up DCB Port, including baud rate
		char ReadBuffer[128];								// Buffer to hold incoming read data
		char WriteBuffer[128];								// Buffer to hold outgoing write data

		void SetError(std::string NewError);				// Basic functionality
		void Initialize();									// Called from Connect() to set up timeouts, overlaps, and DCB
		void FlushBuffer(char* buffer, int bufferSize);		// Basic functionality


		void Disconnect();									// Basic functionality
		int Connect();										// Basic functionality
		std::string GetError();								// Basic functionality
		void ClearErrors();									// Basic functionality
		bool ReadBytes(std::string* Line, int bytesToRead);	// In development
		bool ReadLine(std::string* Line);					// Basic functionality
		bool ReadUntil(std::string* Line, char terminator); // In development
		void Write(std::string* Line, int bytesToWrite);	// Not implemented yet
		bool WriteLine(std::string* Line);					// Basic functionality, need to implement procedure to continue writing if the write isn't complete
		bool isConnected();									// Very basic, needs more implementation or might not be that useful
		bool isValidPort(int COMPortNumber);				// Check if the specified COM port is available/valid
		void ScanPorts();									// Iterate through COM0-COM20 and store valid COM ports in <availablePorts>
		int baudRate;										// Default is 9600
		char* COMPortName;									// Blank by default
		std::vector<std::string> availablePorts;			// Used after calling ScanPorts() to see what ports are available
};

#endif