#include <iostream>
#include "SerialCOM.h"
using namespace std;

int main()
{
	// Initialize new SerialCOM object
	SerialCOM dev;

	//Declare variables
	char portName[32] = { '\0' };
	string buf = "";

	// Scan connected devices to find COM ports
	cout << "Scanning COM ports..." << endl;
	dev.ScanPorts();

	// Ouput open ports
	cout << dev.availablePorts.size() << " COM ports available:" << endl;
	for (int i = 0; i < dev.availablePorts.size(); i++)
	{
		cout << dev.availablePorts[i] << "		";
	}
	cout << endl;

	// Get port number to use
	cout << "Enter COM port number to connect on:" << endl;
	std::getline(std::cin, buf);

	// Set com port
	sprintf(&portName[0], "\\\\.\\COM%d", stoi(buf));
	dev.COMPortName = portName;
	buf = "";
	
	// Get baud rate to use
	cout << "Enter baud rate to connect with:" << endl;
	std::getline(std::cin, buf);
	dev.baudRate = stoi(buf);
	buf = "";


	while (1)
	{
		// Loop until STM device is connected
		cout << "Waiting for connection on: " << dev.COMPortName << "..." << endl;
		while (dev.Connect() == 0)
		{
			Sleep(500);
		}

		cout << "Connected on " << dev.COMPortName << ", attempting to read..." << endl << endl;

		// Drop into a loop to read all incoming data from STM one line at a time
		while (dev.isConnected())
		{

			buf = "";

			if (!dev.ReadLine(&buf))
			{
				cout << endl << "Error encountered: " << dev.GetError() << endl;
				return 1;
			}
			else cout << buf;


		}
	}
	
	cout << "Disconnecting and exiting." << endl;
	dev.Disconnect();

	return 0;
}