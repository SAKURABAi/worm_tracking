/****************************************************************************
    Serial Operations in Linux
****************************************************************************/
#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "QException.h"
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_BUFFER_SIZE 512
#define INVALID_PORT -1

enum SerialStatus{SERIAL_OPENED, SERIAL_CONNECTED, SERIAL_CLOSED};

class QSerial
{
public:
    static string OBJECT_NAME;
	explicit QSerial(const char *name);
	~QSerial();

	void Open_Port();
	void Close_Port();
	inline SerialStatus Get_SerialStatus() const{ return state;} 

	void flush();
	int Read_Port(char *buffer);
	int Write_Port(const char *data);

protected:
    int Read_Char();
	void Initialize(speed_t speed);
	void ClearReadBuffer();
	void ClearWriteBuffer();

private:
	int port;//identifier for serial port
	speed_t baud_rate;
	SerialStatus state;

	char port_name[MAX_BUFFER_SIZE];
	char write_buffer[MAX_BUFFER_SIZE];
	char read_buffer[MAX_BUFFER_SIZE];
};

#endif
