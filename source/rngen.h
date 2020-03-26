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


#ifndef __RNGEN_H
#define	__RNGEN_H


#include <stdint.h>


class Rngen
{
public:

    Rngen (uint32_t s = 0);

    void init (uint32_t s);

    uint32_t irand (void)
    {
	_x = 6364136223846793005 * _x + 1442695040888963407;
	return _x >> 32;
    }

    double  urand (void) { return irand () / _p32d; }
    double  grand (void);
    void    grand (double *x, double *y);
    float   urandf (void) { return irand () / _p32f; }
    float   grandf (void);
    void    grandf (float  *x, float *y);

    ~Rngen (void);
    Rngen (const Rngen&);           // disabled, not to be used
    Rngen& operator=(const Rngen&); // disabled, not to be used

private:

    int init_urandom ();
    
    uint64_t  _x;
    bool      _md;
    bool      _mf;
    double    _bd;
    float     _bf;

    static const double  _p31d;
    static const double  _p32d;
    static const float   _p31f;
    static const float   _p32f;
};


#endif
