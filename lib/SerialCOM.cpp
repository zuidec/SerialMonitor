///
///	SerialCOM is general class used to communicated with COM ports using windows handles
/// and intended for use with microcontrollers such as arduino and stm32
/// 
/// Created by Case Zuiderveld
/// Last updated 3/5/2023
///

#include "SerialCOM.h"
#include <iostream> // Using to debug 

SerialCOM::SerialCOM()
{
	baudRate = 9600;		// Use 9600 as a default baud rate
	comPortName = "";		// Set to empty to prevent random memory assignment

	FlushBuffer(&readBuffer[0], sizeof(readBuffer));	// Clear buffer before we start reading
	FlushBuffer(&writeBuffer[0], sizeof(writeBuffer));	// Clear buffer before we start writing

	SetError("");	// Start with an empty error
}

SerialCOM::~SerialCOM()
{
	Disconnect();
}

void SerialCOM::Initialize()
{
	// When using asynchronous IO, ReadFile() requires the last argument to be a pointer to an overlapped object, which is declared here. We are required to set hEvent, Offset, and OffsetHigh, but there are more properties
	portOverlap.hEvent = NULL;		//
	portOverlap.Offset = 0;			// Needed for asynchronous Read, not currently in use
	portOverlap.OffsetHigh = 0;		//
	//

	// Create a timeout profile with the following parameters
	portTimeouts.ReadIntervalTimeout = 500;			//	500ms between receiving bytes before timing out
	portTimeouts.ReadTotalTimeoutConstant = 100;	//	100ms added to (ReadTotalTimeoutMultiplier * bytes requested) before timing out
	portTimeouts.ReadTotalTimeoutMultiplier = 500;	//	
	portTimeouts.WriteTotalTimeoutConstant = 500;	//
	portTimeouts.WriteTotalTimeoutMultiplier = 500;	//
	
	// Set required properties of DCB object
	memset(&port, 0, sizeof(port));		//
	port.DCBlength = sizeof(port);		//

}

int SerialCOM::Connect()
{
	Initialize();
	commDCBParameter = "baud=" + std::to_string(baudRate) + " parity=n data=8 stop=1";	// Variable to store string containing baud, etc to build DCB with

	try 
	{
		serialPort = CreateFileA (comPortName,	// Set COM port to connect to. If the port is greater than 9, string should be formatted "\\\\.\\COM10" but this format is also valid for ports 1-9
				GENERIC_READ | GENERIC_WRITE,	// Open read and write
				0,								
				NULL,
				OPEN_EXISTING,					// Opens existing COM port
				0,								// Use FILE_FLAG_OVERLAPPED for asynchronous or 0 for synchronous
				NULL);
		
		if(serialPort == INVALID_HANDLE_VALUE) throw 'a';						// throw exception if handle is invalid	
		if (!GetCommState(serialPort, &port)) throw 'c';						// Gets information about serialPort to store in DCB
		if (!BuildCommDCB(commDCBParameter.c_str(), &port)) throw 'd';			// Build DCB according to commDCBParameter variable
		if (!SetCommState(serialPort, &port)) throw 'e';						// Attempt to set serialPort according to build parameters from DCB
		if (!SetCommTimeouts(serialPort, &portTimeouts)) throw 'b';				// Attempt to set timeout profile for arduino and throw exception if unsuccessful
	}
	catch(char a)
	{
		switch(a)		// Catch exceptions and use SetError() to store the correct error
		{
			case 'a':
				SetError("INVALID_HANDLE_ERROR");
				break;
			case 'b':
				SetError("SET_COMM_TIMEOUT_ERROR");
				break;
			case 'c':
				SetError("GET_COMM_STATE_ERROR");
				break;
			case 'd':
				SetError("DCB_BUILD_ERROR");
				break;
			case 'e':
				SetError("SET_COMM_STATE_ERROR");
				break;
			default:
				SetError("WINDOWS_ERROR: " + GetLastError()); // Use GetLastError() to retrieve any other errors not defined
				break;
		}
		
		CloseHandle(serialPort);		// Close handle and exit																			
		return 0;						
	}
	catch (...)
	{
		CloseHandle(serialPort);		// Close handle and exit																			
		return 0;
	}
	return 1;
}

std::string SerialCOM::GetError()
{
	return lastError;
}

void SerialCOM::ClearErrors()
{
	lastError = "";
}

bool SerialCOM::ReadBytes(std::string *line, int bytesToRead)
{
	try
	{
		for(int i=0; i< bytesToRead; i++)		// Read one byte at a time until reached bytesToRead number of bytes
		{
			if (!ReadFile(serialPort, &readBuffer[i], 1, &bytesRead, NULL))
			{
				comError = GetLastError();
				i--;
				switch (comError)
				{
					case ERROR_IO_PENDING:		// We expect to occasionally have IO pending, so ignore
						break;
					case ERROR_HANDLE_EOF:		// We expect to occasionally read to the end of the buffer, so ignore
						break;
					default:					// For all other errors, we should throw an exception
						throw comError;
						break;
				}
			}
			if (i == (bytesToRead - 1))
			{
				for (int j = 0; j <= i; j++)
				{
					*line += readBuffer[j];		// Convert buffer into a string to be returned							
				}
			}
		}
		FlushBuffer(&readBuffer[0], sizeof(readBuffer));
	}
	catch (...) //Catch exceptions caused by ReadFile() and exit
	{
		SetError("READ_ERROR " + std::to_string(comError)); // Output last error and exit 
		Disconnect();
		return false;
	}

	return true;
}

bool SerialCOM::ReadLine(std::string* line) // Takes a string pointer as argument to store the line to be read
{
	bool EndOfFile = false;		// Create an EOF flag to use in this function
	try		
	{
		for (int i = 0; !EndOfFile&& i<sizeof(readBuffer); i++) // Set up loop to read until \n is found
		{
			if (!ReadFile(serialPort, &readBuffer[i], 1, &bytesRead, /*&portOverlap*/ NULL)) // Read one byte at a time from arduino and store in buffer, ReadFile() returns 0 if there is a failure. Use portOverlap as last argument for asynchronous IO, NULL for synchronous IO
			{
				comError = GetLastError();
				i--;
				switch (comError)
				{
				case ERROR_IO_PENDING:		// We expect to occasionally have IO pending, so ignore
					break;
				case ERROR_HANDLE_EOF:		// We expect to occasionally read to the end of the buffer, so ignore
					break;
				default:					// For all other errors, we should throw an exception
					throw comError;
					break;

				}

			}
			if (bytesRead < 1) i--;

			if (readBuffer[i] == '\n' && i > 0)
			{
				EndOfFile = true;				// Set EOF flag if terminating character is reached
				for (int j = 0; j <= i; j++)
				{
					*line += readBuffer[j];		// Convert buffer into a string to be returned							
				}
				FlushBuffer(&readBuffer[0], sizeof(readBuffer));		// Send the buffer to be set back to \0 in all positions
			}
		}
		
		EndOfFile = false; // Reset EOF flag
	}
	catch (...) //Catch exceptions caused by ReadFile() and exit
	{
		SetError("READ_ERROR " + std::to_string(comError)); // Output last error and exit 
		Disconnect();
		return false;
	}
	return true;
}

bool SerialCOM::ReadUntil(std::string* line, char terminator) // Takes a string pointer as argument to store the line to be read
{
	bool EndOfFile = false;		// Create an EOF flag to use in this function
	try
	{
		for (int i = 0; !EndOfFile && i < sizeof(readBuffer); i++) // Set up loop to read until \n is found
		{
			if (!ReadFile(serialPort, &readBuffer[i], 1, &bytesRead, /*&portOverlap*/ NULL)) // Read one byte at a time from arduino and store in buffer, ReadFile() returns 0 if there is a failure. Use portOverlap as last argument for asynchronous IO, NULL for synchronous IO
			{
				comError = GetLastError();
				i--;
				switch (comError)
				{
				case ERROR_IO_PENDING:		// We expect to occasionally have IO pending, so ignore
					break;
				case ERROR_HANDLE_EOF:		// We expect to occasionally read to the end of the buffer, so ignore
					break;
				default:					// For all other errors, we should throw an exception
					throw comError;
					break;

				}

			}
			if (bytesRead < 1) i--;

			if (readBuffer[i] == terminator && i > 0)
			{
				EndOfFile = true;				// Set EOF flag if terminating character is reached
				for (int j = 0; j <= i; j++)
				{
					*line += readBuffer[j];		// Convert buffer into a string to be returned							
				}
				FlushBuffer(&readBuffer[0], sizeof(readBuffer));		// Send the buffer to be set back to \0 in all positions
			}
		}

		EndOfFile = false; // Reset EOF flag
	}
	catch (...) //Catch exceptions caused by ReadFile() and exit
	{
		SetError("READ_ERROR " + std::to_string(comError)); // Output last error and exit 
		Disconnect();
		return false;
	}
	return true;
}

void SerialCOM::Write(std::string *line, int bytesToWrite)
{
	
}

bool SerialCOM::WriteLine(std::string* line)
{
	for (int i = 0; i< line->size(); i++)
	{ 
		writeBuffer[i] = line->at(i);			// Write the data from the string pointer into local buffer
	}
	writeBuffer[line->size()] = '\n';			// Null terminate the buffer
	bytesToWrite = line->size() +1;				// Set bytesToWrite equal to the size of line +1 to accomodate for the addition of null terminator
	
	bytesWritten = 0;							// Reset the number of bytes written

	try
	{
		
		if (!WriteFile(serialPort, &writeBuffer[0], bytesToWrite, &bytesWritten, NULL))		// Attempt to write bytesToWrite number of bytes
		{
			comError = GetLastError();
			
			switch (comError)
			{
			case ERROR_IO_PENDING:		// We expect to occasionally have IO pending, so ignore
				break;
			default:					// For all other errors, we should throw an exception
				throw comError;
				break;

			}
		}
		else if (bytesWritten != bytesToWrite)
		{
			throw 'a';
		}
		
	}
	catch (char a)
	{
		if (a == 'a')	// Catch exception caused by WriteFile() not writing all the data from the buffer and disconnect
		{
			SetError("WRITE_INCOMPLETE " + std::to_string(bytesToWrite - bytesWritten) + " BYTES NOT WRITTEN");
			Disconnect();
			return false;
		}
	}
	catch (...) //Catch all other exceptions caused and exit
	{
		SetError("WRITE_ERROR " + std::to_string(comError)); // Output last error and disconnect
		Disconnect();
		return false;
	}
	FlushBuffer(&writeBuffer[0], sizeof(writeBuffer));		// Clear out buffer before exiting
	return true;
}

void SerialCOM::SetError(std::string newError)
{
	lastError = newError;
}

void SerialCOM::Disconnect()
{
	FlushBuffer(&readBuffer[0], sizeof(readBuffer));
	FlushBuffer(&writeBuffer[0], sizeof(writeBuffer));
	CloseHandle(serialPort);
}

void SerialCOM::FlushBuffer(char *buffer, int bufferSize)
{
	for (int i = 0; i < bufferSize; i++)
	{
		buffer[i] = '\0';			// Reset all members of buffer to \0
	}

	return;
}

bool SerialCOM::isValidPort(int comPortNumber)
{
	if (comPortNumber < 0){
		SetError("INVALID_PORT_NAME");									// Set an invalid port name error if value is less than 0
		return false;
	}

	char testPort[12] = "\\\\.\\COM";
	snprintf(testPort,sizeof(testPort),"\\\\.\\COM%d", comPortNumber);	// Add passed port number into string
	char* tempPort = comPortName;										// Store the current port so we can change it back later
	
	comPortName = testPort;												// Set to our port to be tested
	if (Connect()==0) {													// Attempt to connect
		Disconnect();			
		comPortName = tempPort;											// Restore the port value and exit
		SetError("INVALID_COM_PORT");
		return false;
	}
	else {
		Disconnect();
		comPortName = tempPort;											// Restore the port value and exit
		return true;
	}
	
}

bool SerialCOM::isConnected()
{
	if (serialPort == INVALID_HANDLE_VALUE) return false;
	else return true;

}

void SerialCOM::ScanPorts()
{
	availablePorts.clear();		// Clear out the vector before we begin the scan

	for (int i = 0; i <= 20; i++)	// Scan ports 0-20
	{
		if (isValidPort(i))			// Use isValidPort to let us know if something is there
		{	
			availablePorts.push_back("\\\\.\\COM" + std::to_string(i));		// Store valid ports into vector
		}
	}

	if (GetError() == "INVALID_COM_PORT") ClearErrors();		// Reset errors caused by scanning ports

}