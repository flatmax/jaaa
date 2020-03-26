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


#ifndef __AUDIO_H
#define __AUDIO_H


#include <stdlib.h>
#include <math.h>
#include <zita-alsa-pcmi.h>
#include <clthreads.h>
#include <jack/jack.h>
#include "rngen.h"


class Audio : public A_thread
{
public:

    Audio (ITC_ctrl *cmain, const char *name);
    virtual ~Audio (void);

    void  init_alsa (const char *playdev, const char *captdev,
                     int fsamp, int frsize, int nfrags, int ncapt, int nplay);
    void  init_jack (const char *server);

private:

    enum { LSINE = 4096, LRAND = 4096, MRAND = LRAND - 1 };

    virtual void thr_main (void);
    void  init (void);
    void  process (void);
    void  generate (int size);
    void  close_alsa (void);
    void  close_jack (void);
    void  jack_shutdown (void);
    int   jack_callback (jack_nframes_t nframes);

    const char     *_jname;
    ITC_ctrl       *_cmain;

    volatile bool   _run_alsa;
    Alsa_pcmi      *_alsa_handle;

    volatile bool   _run_jack;
    jack_client_t  *_jack_handle;
    jack_port_t    *_jack_in  [8];
    jack_port_t    *_jack_out [8];

    bool           _active;
    unsigned long  _fsamp;
    unsigned long  _fsize;
    int            _ncapt;
    int            _nplay;
    int            _input;
    float         *_data;
    int            _dind;
    int            _size;
    int            _step;
    int            _scnt;
    float          _sine [LSINE + 1];
    float         *_outs;
    int            _g_bits;
    float          _a_noise;
    float          _a_sine1;
    float          _f_sine1;
    float          _p_sine1;
    float          _a_sine2;
    float          _f_sine2;
    float          _p_sine2;
    Rngen          _rngen;
  
    static void jack_static_shutdown (void *arg);
    static int  jack_static_callback (jack_nframes_t nframes, void *arg);
};


#endif
