///
///	SerialCOM is general class used to communicated with COM ports using windows handles
/// and intended for use with microcontrollers such as arduino and stm32
/// 
/// Created by Case Zuiderveld
/// Last updated 3/5/2023
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

		void Disconnect();									// Basic functionality
		int Connect();										// Basic functionality
		std::string GetError();								// Basic functionality
		void ClearErrors();									// Basic functionality
		bool ReadBytes(std::string* line, int bytesToRead);	// In development
		bool ReadLine(std::string* line);					// Basic functionality
		bool ReadUntil(std::string* line, char terminator); // In development
		void Write(std::string* line, int bytesToWrite);	// Not implemented yet
		bool WriteLine(std::string* line);					// Basic functionality, need to implement procedure to continue writing if the write isn't complete
		bool isConnected();									// Very basic, needs more implementation or might not be that useful
		bool isValidPort(int comPortNumber);				// Check if the specified COM port is available/valid
		void ScanPorts();									// Iterate through COM0-COM20 and store valid COM ports in <availablePorts>
		int baudRate;										// Default is 9600
		char* comPortName;									// Blank by default
		std::vector<std::string> availablePorts;			// Used after calling ScanPorts() to see what ports are available
		
	private:
		DWORD comError;										// Variable to store the results of the windows GetLastError() as needed
		DWORD bytesRead;									// ReadFile requires a pointer to a DWORD to output bytes read. This will be set back to 0 if EOF is reached 
		DWORD bytesWritten;									// WriteFile requires this to output bytes written
		DWORD bytesToWrite;									// Tells WriteFile() how many bytes to write
		HANDLE serialPort;									// The handle used to access the serial port
		DCB port;											// Holds parameters used to set up serial port handle
		OVERLAPPED portOverlap;								// Needed to use asynchronously with ReadFile, not currently used
		COMMTIMEOUTS portTimeouts;							// Holds all the timeout information for the handle
		std::string lastError;								// Class specific variable to store class defined errors
		std::string commDCBParameter;						// Contains required data to set up DCB Port, including baud rate
		char readBuffer[128];								// Buffer to hold incoming read data
		char writeBuffer[128];								// Buffer to hold outgoing write data

		void SetError(std::string newError);				// Basic functionality
		void Initialize();									// Called from Connect() to set up timeouts, overlaps, and DCB
		void FlushBuffer(char* buffer, int bufferSize);		// Basic functionality
};

#endif