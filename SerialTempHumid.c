//1�ʸ��� �µ�,������ üũ�� Serial Port�� �����ִ� ���α׷�
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "wiringPi.h"
#include "wiringPiI2C.h"
#include "wiringSerial.h"


#define ADDR 0x40

double cTemp(int sensorTemp)
{
	double tSensorTemp = sensorTemp / 65536.0;
	return -46.85 + (175.72 * tSensorTemp);
}

double cHumid(int sensorHumid)
{
	double tSensorHumid = sensorHumid / 65536.0;
	return -6.0 + (125.0 * tSensorHumid);
}

int main(void)
{
	int fd, fd2;
	unsigned char charbuffer[10] = { 0, }; //ȭ�鿡 ����� ���ڿ��� ����� ����
	unsigned int x;
	unsigned char buf[4];
	unsigned int temp, humid;

	system("gpio load i2c");

	if (wiringPiSetup() == -1)  //wiringPi�� �ʱ�ȭ�Ѵ�.
		return 1;

	if ((fd = wiringPiI2CSetup(ADDR)) < 0)
	{
		fprintf(stderr, "Unable to open I2C device: %s\n", strerror(errno));
		exit(-1);
	}

	if ((fd2 = serialOpen("/dev/ttyAMA0", 115200)) < 0)
	{
		fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
		exit(-1);
	}


	system("clear");

	while (1) {

		wiringPiI2CWrite(fd, 0xF3);
		delay(100);
		x = read(fd, buf, 3);

		if (x != 3)
			printf("%4d %02X %02X %02X\n", x, buf[0], buf[1], buf[2]);

		temp = (buf[0] << 8 | buf[1]) & 0xFFFC;

		printf("%4.1fC\n", cTemp(temp));
		sprintf(charbuffer, "%4.1fC\n\r", cTemp(temp)); //���� ����� ȭ���� �ƴ� ���ڿ� ���ۿ� ����
		serialPrintf(fd2, charbuffer); // uart�� ���ڿ� ������ ������ ����


		wiringPiI2CWrite(fd, 0xF5);
		delay(100);
		x = read(fd, buf, 3);


		if (x != 3)
			printf("%4d %02X %02X %02X\n", x, buf[0], buf[1], buf[2]);
		humid = (buf[0] << 8 | buf[1]) & 0xFFFC;

		printf("%5.2f %%rh\n", humid, cHumid(humid));
		sprintf(charbuffer, "%5.2f %%rh\n\r", cHumid(humid)); //���� ����� ȭ���� �ƴ� ���ڿ� ���ۿ� ����
		serialPrintf(fd2, charbuffer); // uart�� ���ڿ� ������ ������ ����

		delay(1000);
		system("clear");
	}
	return 0;

}

