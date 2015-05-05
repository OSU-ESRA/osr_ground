/***************************************************************************//**
 * @file example/MainWindow.cpp
 * @author  Marek M. Cel <marekcel@mscsim.org>
 *
 * @section LICENSE
 *
 * Copyright (C) 2013 Marek M. Cel
 *
 * This file is part of QFlightInstruments. You can redistribute and modify it
 * under the terms of GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Further information about the GNU General Public License can also be found
 * on the world wide web at http://www.gnu.org.
 *
 * ---
 *
 * Copyright (C) 2013 Marek M. Cel
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 ******************************************************************************/
#ifndef MAINWINDOW_CPP
#define MAINWINDOW_CPP
#endif

////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <thread>

#include "MainWindow.h"
#include "ui_MainWindow.h"

////////////////////////////////////////////////////////////////////////////////

float alpha     =  0.0f;
float beta      =  0.0f;
float roll      =  0.0f;
float pitch     =  0.0f;
float heading   =  0.0f;
float slipSkid  =  0.0f;
float turnRate  =  0.0f;
float devH      =  0.0f;
float devV      =  0.0f;
float airspeed  =  0.0f;
float altitude  =  0.0f;
float pressure  = 28.0f;
float climbRate =  0.0f;
float machNo    =  0.0f;

float prev_alt;
int prev_time;

using namespace std;

////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow( QWidget * parent ) :
    QMainWindow( parent ),
    m_ui( new Ui::MainWindow ),

    m_timerId ( 0 ),
    m_steps   ( 0 ),

    m_realTime ( 0.0 )
{
	
    m_ui->setupUi( this );
    
    std::thread t1(&MainWindow::updateValues, this);
    t1.detach();

    m_timerId  = startTimer( 0 );

    m_time.start();
}

////////////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow()
{
    cout << "Average time step: " << ( (double)m_realTime ) / ( (double)m_steps ) << " s" << endl;

    if ( m_timerId ) killTimer( m_timerId );

    if ( m_ui ) delete m_ui; m_ui = 0;
}

///////////////////////////////////////////////////////////////////////////////


void MainWindow::handleMessage(string input) {
	std::vector<std::string> tokens;
	boost::split(tokens, input, boost::is_any_of(","));
	
	if (tokens[0].compare("attitude") == 0) {
		std::cout << "Attitude: ";
		
		float dcm[9];
		for (int i = 0; i < 9; i++) {
			dcm[i] = atof(tokens[i+1].c_str());
			std::cout << dcm[i] << ", ";
		}
		std::cout << std::endl;

		roll = 0.0;//atan2(dcm[7], dcm[8]) * 180 / M_PI;
		pitch = 0.0;//-asin(dcm[6]) * 180 / M_PI;
		heading = atan2(-dcm[1], dcm[0]) * 180 / M_PI;
	}
	else if (tokens[0].compare("altitude") == 0) {
		altitude = atof(tokens[1].c_str());
		int time = atoi(tokens[2].c_str());
		std::cout << "Altitude: " << altitude << ", time: " << time << "\n";
		float elapsed = ((float)time - prev_time) / 1000.0;
		climbRate = (altitude - prev_alt) / elapsed;
		machNo = climbRate / 343.59;
		airspeed = abs(climbRate);
		
		prev_alt = altitude;
		prev_time = time;
	}
	else if (tokens[0].compare("pressure") == 0) {
		//pressure = atof(tokens[1].c_str());
		//std::cout << "Got pressure message:" << pressure << std::endl;
	}
}


void MainWindow::updateValues() {
	int pipein = open("/tmp/rocket_instrument", O_RDONLY);
	char buffer[200];
	
	while (true) {
		memset(buffer, 0, 200);
		int readin = read(pipein, buffer, sizeof(buffer));
		if (readin > 0) {
			string input(buffer);
			handleMessage(input);
		}
	}
}


void MainWindow::timerEvent( QTimerEvent * event )
{
	/////////////////////////////////
	QMainWindow::timerEvent( event );
	/////////////////////////////////
	
	float timeStep = m_time.restart();
    
    m_realTime = m_realTime + timeStep / 1000.0f;

    m_ui->widgetPFD->setFlightPathMarker ( alpha, beta );
    m_ui->widgetPFD->setRoll          ( roll     );
    m_ui->widgetPFD->setPitch         ( pitch     );
    m_ui->widgetPFD->setSlipSkid      ( slipSkid  );
    m_ui->widgetPFD->setTurnRate      ( turnRate / 3.0f );
    m_ui->widgetPFD->setDevH          ( devH      );
    m_ui->widgetPFD->setDevV          ( devV      );
    m_ui->widgetPFD->setHeading       ( heading   );
    m_ui->widgetPFD->setAirspeed      ( airspeed  );
    m_ui->widgetPFD->setMachNo        ( machNo    );
    m_ui->widgetPFD->setAltitude      ( altitude  );
    m_ui->widgetPFD->setPressure      ( pressure  );
    m_ui->widgetPFD->setClimbRate     ( climbRate / 100.0f );

    m_ui->widgetPFD->update();

    m_steps++;
}
