// ----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2018 Fons Adriaensen <fons@linuxaudio.org>
//    
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------


#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "rngen.h"


const double Rngen::_p31d = 2147483648.0;
const double Rngen::_p32d = 4294967296.0;
const float Rngen::_p31f = 2147483648.0f;
const float Rngen::_p32f = 4294967296.0f;


Rngen::Rngen (uint32_t s)
{
    init (s);
}

 
Rngen::~Rngen (void)
{
}

 
int Rngen::init_urandom (void)
{
    int   n, k, fd;
    char *p;
    
    fd = open ("/dev/urandom", O_RDONLY);
    if (fd < 0) return 1;
    for (n = 8, p = (char *) &_x; n; n -= k, p += k)
    {	
        k = read (fd, p, n);
	if (k < 0)
	{
	    close (fd);
	    return 1;
	}
    }
    close (fd);
    return 0;
}


void Rngen::init (uint32_t s)
{
    int i;

    _md = false;
    _mf = false;
    if (s == 0)
    {
	if (init_urandom ()) s = time (0);
        else return;
    }
    for (i = 0; i < 100; i++)
    {
	s = 1103515245 * s + 12345;
    }
    _x = 0;
    for (i = 0; i < 8; i++)
    {
	_x = (_x << 8) + (s >> 24);
	s = 1103515245 * s + 12345;
    }
}

 
double Rngen::grand (void)
{
    double a, b, r;

    if (_md)
    {
	_md = false;
	return _bd;
    }
    do
    {
	a = irand () / _p31d - 1.0;
	b = irand () / _p31d - 1.0;
	r = a * a + b * b;
    }
    while ((r > 1.0) || (r < 1e-40));
    r = sqrt (-2 * log (r) / r);
    _md = true;
    _bd = r * b;
    return r * a;
}    

 
void Rngen::grand (double *x, double *y)
{
    double a, b, r;

    do
    {
	a = irand () / _p31d - 1.0;
	b = irand () / _p31d - 1.0;
	r = a * a + b * b;
    }
    while ((r > 1.0) || (r < 1e-40));
    r = sqrt (-log (r) / r);
    *x = r * a;
    *y = r * b;
}    


float Rngen::grandf (void)
{
    float a, b, r;

    if (_mf)
    {
	_mf = false;
	return _bf;
    }
    do
    {
	a = irand () / _p31f - 1.0f;
	b = irand () / _p31f - 1.0f;
	r = a * a + b * b;
    }
    while ((r > 1.0f) || (r < 1e-20f));
    r = sqrtf (-2 * logf (r) / r);
    _mf = true;
    _bf = r * b;
    return r * a;
}    

 
void Rngen::grandf (float *x, float *y)
{
    float a, b, r;

    do
    {
	a = irand () / _p31f - 1.0f;
	b = irand () / _p31f - 1.0f;
	r = a * a + b * b;
    }
    while ((r > 1.0f) || (r < 1e-20f));
    r = sqrtf (-logf (r) / r);
    *x = r * a;
    *y = r * b;
}    
