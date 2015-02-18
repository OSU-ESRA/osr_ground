/*
 * Log incoming serial GPS data in KML format that Google Earth
 * can read as a realtime GPS signal.
 *
 * Copyright (c) 1997-2009 Diomidis Spinellis
 *
 * Based on code:
 * Copyright (C) 2007 by Jaroslaw Zachwieja <grok!warwick.ac.uk>
 * Copyright (C) 2008 by TJ <linux!tjworld.net>
 *
 * Published under the terms of GNU General Public License v2 or later.
 * License text available at http://www.gnu.org/licenses/licenses.html#GPL
 *
 * $Id: logkml.c,v 1.4 2009/07/01 13:55:37 dds Exp $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__MACH__) || defined(__FreeBSD__) || defined(__linux__)
#define UNIX
#endif

#if defined(_WIN32)
#include <windows.h>

typedef HANDLE file_handle_type;
typedef DWORD bytes_read_type;
#elif defined(UNIX)
#include <fcntl.h>

typedef int file_handle_type;
typedef int bytes_read_type;
#else
#error "No definition for file handle type. Unknown Unix system?"
#endif

/*
 * Format the last OS error as a human-readable error
 * message, print it to stderr, and exit the program.
 */
static void
error(const char *s)
{
#if defined(_WIN32)
	LPVOID lpMsgBuf;
	char buff[1024];

	FormatMessage(
	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_FROM_SYSTEM |
	    FORMAT_MESSAGE_IGNORE_INSERTS,
	    NULL,
	    GetLastError(),
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	    (LPTSTR) &lpMsgBuf,
	    0,
	    NULL
	);
	fprintf(stderr, "%s: %s\n", s, lpMsgBuf);
	LocalFree(lpMsgBuf);
#elif defined(UNIX)
	perror(s);
#else
#error "No definition for error printing. Unknown Unix system?"
#endif
	exit(1);
}

/*
 * Read and return a line terminated by \r or \n from fd.
 * Lines are truncated to 1k characters.
 * Copy the characters read to the standard output.
 */
static char *
readline(file_handle_type fd)
{
	bytes_read_type n;
	int i;
	char buff[256];
	time_t t;
	static char rbuff[1024];
	int rbuff_i = 0;

	for (;;) {
#if defined(_WIN32)
		if (!ReadFile(fd, buff, 1, &n, NULL))
#elif defined(UNIX)
		/*
		 * More optimal to set to non-blocking, do a select
		 * and then read into a large buffer.
		 */
		if ((n = read(fd, buff, 1)) < 0)
#else
#error "No read line implementation. Unknown Unix system?"
#endif
			error("read");
		for (i = 0; i < n; i++) {
			if (rbuff_i < sizeof(rbuff) - 1)
				rbuff[rbuff_i++] = buff[i];
			if (rbuff_i == sizeof(rbuff) - 1) {
				rbuff[rbuff_i] = 0;
				return rbuff;
			}
			putchar(buff[i]);
			if (buff[i] == '\r' || buff[i] == '\n') {
				rbuff[rbuff_i] = 0;
				return rbuff;
			}
		}
	}
}


int
main(int argc, char **argv)
{
	FILE *f;
	char buff[1024];
	int i;
	file_handle_type serialfd;
	bytes_read_type n;
	char *s;
#if defined(_WIN32)
	DCB dcb;
	COMMTIMEOUTS ctm;
#endif

	if (argc != 4) {
		fprintf(stderr, "usage: %s port settings log-file\n", argv[0]);
#if defined(_WIN32)

		fprintf(stderr, "Example: %s COM11 \"57600,n,8,1\" \"C:/Documents and Settings/dds/Application Data/Google/GoogleEarth/realtime/Realtime GPS.kml\"\n", argv[0]);
#elif defined(__APPLE__)
		fprintf(stderr, "Example: %s tty.BTGPS-SPPSlave-1 '' /Users/dds/Documents/Google/GoogleEarth/realtime/Realtime GPS.kml\"\n", argv[0]);
#endif
		exit(1);
	}

#if defined(_WIN32)
	sprintf(buff, "\\\\.\\%s", argv[1]);
	serialfd = CreateFile(buff,  GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (serialfd == INVALID_HANDLE_VALUE)
		error("CreateFile");

	FillMemory(&dcb, sizeof(dcb), 0);
	dcb.DCBlength = sizeof(dcb);
	if (!BuildCommDCB(argv[2], &dcb))
		error("BuildCommDCB");
	if (!SetCommState(serialfd, &dcb))
		error("SetCommState");

	if (!GetCommTimeouts(serialfd, &ctm))
		error("GetCommTimeouts");
	ctm.ReadIntervalTimeout = 60000;	// 1 min
	ctm.ReadTotalTimeoutMultiplier = ctm.ReadTotalTimeoutConstant = 0;
	if (!SetCommTimeouts(serialfd, &ctm))
		error("SetCommTimeouts");
#elif defined(UNIX)
	sprintf(buff, "/dev/%s", argv[1]);
	serialfd = open(buff, O_RDWR, 0);
	if (serialfd < 0) {
		perror(buff);
		exit(1);
	}
#else
#error "No way to open a serial device"
#endif

	/* Read and log loop */
	for (;;) {
		/*
		 * Parse a line of the following format
		 * $GPRMC,110311.000,A,3804.1121,N,02349.2643,E,23.09,355.37,260108,,,A*5C
		 * 0      1          2 3         4 5          6 7     8      9
		  *       time         lat         lon          speed heading
		 */
		char *s, *val;
		double latitude_in, longitude_in, altitude_in, speed_in, heading_in;
		double latitude_degrees, longitude_degrees;
		double latitude_minutes, longitude_minutes;
		double longitude, latitude;
		double speed, tilt, heading, range;

		s = readline(serialfd);
		if (strcmp(strtok(s, ","), "$GPRMC"))	// 0
			continue;
		if (strtok(NULL, ",") == NULL)		// 1
			continue;
		if (strtok(NULL, ",") == NULL)		// 2
			continue;
		if ((val = strtok(NULL, ",")) == NULL)	// 3
			continue;
		latitude_in = atof(val);
		if ((val = strtok(NULL, ",")) == NULL)	// 4
			continue;
		if (*val == 'S')
			latitude_in = -latitude_in;
		if ((val = strtok(NULL, ",")) == NULL)	// 5
			continue;
		longitude_in = atof(val);
		if ((val = strtok(NULL, ",")) == NULL)	// 6
			continue;
		if (*val == 'W')
			longitude_in = -longitude_in;
		if ((val = strtok(NULL, ",")) == NULL)	// 7
			continue;
		speed_in = atof(val);
		if ((val = strtok(NULL, ",")) == NULL)	// 8
			continue;
		heading_in = atof(val);

		latitude_degrees = (int)(latitude_in / 100);
		latitude_minutes = latitude_in - latitude_degrees * 100;

		longitude_degrees = (int)(longitude_in / 100);
		longitude_minutes = longitude_in - longitude_degrees * 100;

		latitude = latitude_degrees + (latitude_minutes / 60);
		longitude = longitude_degrees + (longitude_minutes / 60);

		speed = (int)(speed_in * 1.852);
		range = (( speed / 100 ) * 350) + 650;
		tilt = (( speed / 120 ) * 43) + 30;
		heading = heading_in;

		if (speed < 10) {
			range = 200;
			tilt = 30;
			heading = 0;
		}

		if ((f = fopen(argv[3], "w")) == NULL) {
			perror(buff);
			exit(1);
		}
		fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<kml xmlns=\"http://earth.google.com/kml/2.0\">\n"
		"<Placemark>\n"
		"<name>%.0f km/h</name>\n"
		"<description>^</description>\n"
		"<LookAt>\n"
		"<longitude>%f</longitude>\n"
		"<latitude>%f</latitude>\n"
		"<range>%f</range>\n"
		"<tilt>%f</tilt>\n"
		"<heading>%f</heading>\n"
		"</LookAt>\n"
		"<Point>\n"
		"<coordinates>%f,%f,0</coordinates>\n"
		"</Point>\n"
		"</Placemark>\n"
		"</kml>\n",
		speed, longitude, latitude, range, tilt, heading,
		longitude, latitude);
		fclose(f);
	}
}
