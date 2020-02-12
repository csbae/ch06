//1초마다 온도,습도를 체크후 Serial Port로 보내주는 프로그램
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
	unsigned char charbuffer[10] = { 0, }; //화면에 출력할 문자열이 저장될 버퍼
	unsigned int x;
	unsigned char buf[4];
	unsigned int temp, humid;

	system("gpio load i2c");

	if (wiringPiSetup() == -1)  //wiringPi를 초기화한다.
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
		sprintf(charbuffer, "%4.1fC\n\r", cTemp(temp)); //문자 출력을 화면이 아닌 문자열 버퍼에 저장
		serialPrintf(fd2, charbuffer); // uart로 문자열 버퍼의 내용을 전송


		wiringPiI2CWrite(fd, 0xF5);
		delay(100);
		x = read(fd, buf, 3);


		if (x != 3)
			printf("%4d %02X %02X %02X\n", x, buf[0], buf[1], buf[2]);
		humid = (buf[0] << 8 | buf[1]) & 0xFFFC;

		printf("%5.2f %%rh\n", humid, cHumid(humid));
		sprintf(charbuffer, "%5.2f %%rh\n\r", cHumid(humid)); //문자 출력을 화면이 아닌 문자열 버퍼에 저장
		serialPrintf(fd2, charbuffer); // uart로 문자열 버퍼의 내용을 전송

		delay(1000);
		system("clear");
	}
	return 0;

}

