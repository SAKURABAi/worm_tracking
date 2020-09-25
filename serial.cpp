
#include "serial.h"

string QSerial::OBJECT_NAME = "QSerial";

QSerial::QSerial(const char *name)
{
	strncpy(port_name, name, MAX_BUFFER_SIZE);
	port = INVALID_PORT;
	state = SERIAL_CLOSED;
}

QSerial::~QSerial()
{
    Close_Port();
}

void QSerial::Open_Port()
{
	if((port = open(port_name, O_RDWR|O_NOCTTY)) == -1){
		throw QException("Fail to open serial port");
	}
	
	Initialize(B9600);
	state = SERIAL_OPENED;
}

void QSerial::Initialize(speed_t speed)
{
	struct termios Opt;
	tcgetattr(port, &Opt);

	//Set baud rate
	cfsetospeed(&Opt, speed);
    cfsetispeed(&Opt, speed);

	Opt.c_cflag &= ~(PARENB | CSTOPB | CSIZE); 
    Opt.c_cflag |= CLOCAL | CREAD | CS8;
    Opt.c_oflag &= ~(OPOST);
    tcsetattr(port, TCSANOW, &Opt);
	fcntl(port, F_SETFL, FNDELAY);//Set to immediate (no line control)
	state = SERIAL_CONNECTED;
}

void QSerial::Close_Port()
{
	if(port != INVALID_PORT){
		close(port);
		port = INVALID_PORT;
	}
	state = SERIAL_CLOSED;
}

void QSerial::ClearReadBuffer()
{
	memset(read_buffer, 0, sizeof(read_buffer));
}

void QSerial::ClearWriteBuffer()
{
	memset(write_buffer, 0, sizeof(write_buffer));
}

int QSerial::Write_Port(const char *data)
{
    int write_datasize;
    
    ClearWriteBuffer();
    sprintf(write_buffer, "%s", data);
    write_datasize = strlen(write_buffer);
	write(port, write_buffer, write_datasize);//向串口写数据
	
	return write_datasize;//返回写数据的大小
}

int QSerial::Read_Char()
{
   int n = 0;
   char	c;
   
   if (port == INVALID_PORT){return(-1);}
   n = read(port, &c, 1);
   return((n > 0) ? (c & 0xff):n);
}

int QSerial::Read_Port(char *buffer)
{    
    return read(port, buffer, MAX_BUFFER_SIZE);
}

void QSerial::flush()
{
	tcflush(port, TCIOFLUSH);
}
