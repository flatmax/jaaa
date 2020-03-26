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
#include "audio.h"
#include "messages.h"


Audio::Audio (ITC_ctrl *cmain, const char *name) :
    A_thread ("Audio"),
    _jname (name),
    _cmain (cmain),
    _run_alsa (0),
    _run_jack (0),
    _active (false),
    _ncapt (8),
    _nplay (8),
    _input (-1),
    _data (0),
    _outs (0)
{
}


Audio::~Audio (void)
{
    if (_run_alsa) close_alsa ();
    if (_run_jack) close_jack ();
    delete[] _outs;
}


void Audio::init (void)
{
    int i;

    for (i = 0; i < LSINE; i++)
    {
        _sine [i] = sin (2 * M_PI * i / LSINE);
    }
    _sine [LSINE] = 0;
    _g_bits = 0;
    _a_noise = 0.0;
    _a_sine1 = 0.0;
    _f_sine1 = 0.0;
    _p_sine1 = 0.0;
    _a_sine2 = 0.0;
    _f_sine2 = 0.0;
    _p_sine2 = 0.0;
}


void Audio::init_alsa (const char *playdev, const char *captdev, 
                       int fsamp, int fsize, int nfrags, int ncapt, int nplay)
{
    _run_alsa = true;
    _alsa_handle = new Alsa_pcmi (playdev, captdev, 0, fsamp, fsize, nfrags);
    if (_alsa_handle->state () < 0)
    {
        fprintf (stderr, "Can't connect to ALSA\n");
        exit (1);
    } 
    _ncapt = _alsa_handle->ncapt ();
    _nplay = _alsa_handle->nplay ();
    _fsamp = fsamp;
    _fsize = fsize;

    _outs = new float [fsize];
    init ();
   
    _cmain->put_event (EV_MESG, new M_jinfo (fsamp, fsize, _jname));
    _alsa_handle->printinfo ();
    fprintf (stderr, "Connected to ALSA with %d inputs and %d outputs\n", _ncapt, _nplay); 

    if (thr_start (SCHED_FIFO, -10, 0x00010000))
    {
        fprintf (stderr, "Can't create ALSA thread with RT priority\n");
        if (thr_start (SCHED_OTHER, 0, 0x00010000))
        {
            fprintf (stderr, "Can't create ALSA thread\n");
            exit (1);
	}
    }
}


void Audio::close_alsa ()
{
//    fprintf (stderr, "Closing ALSA...\n");
    _run_alsa = false;
    get_event (1 << EV_EXIT);
    delete _alsa_handle;
}


void Audio::thr_main (void) 
{
    unsigned long b, k, m, n;
    int  i;

    _alsa_handle->pcm_start ();

    while (_run_alsa)
    {
	k = _alsa_handle->pcm_wait ();  
        while (k >= _fsize)
       	{
            if (_ncapt)
	    { 
		_alsa_handle->capt_init (_fsize);

		if (_data)
		{
  	            m = _fsize;
		    n = _size - _dind;
		    if (m >= n)
		    {
			if (_input < 0) memset (_data + _dind, 0, n * sizeof (float));
			else _alsa_handle->capt_chan (_input, _data + _dind, n);
			_dind = 0;
			m -= n;
		    }
		    if (m)
		    {
			if (_input < 0) memset (_data + _dind, 0, m * sizeof (float));
			else _alsa_handle->capt_chan (_input, _data + _dind, m);
			_dind += m;
		    }
		}

		_alsa_handle->capt_done (_fsize);
	    }

            if (_nplay)
	    {
		generate (_fsize);
                b = _g_bits & 15;
		_alsa_handle->play_init (_fsize);

		for (i = 0; i < _nplay; i++, b >>= 1)
		{
		    if (b & 1) _alsa_handle->play_chan (i, _outs, _fsize);
		    else       _alsa_handle->clear_chan (i, _fsize);
		}               

		_alsa_handle->play_done (_fsize);
	    }

            k -= _fsize;
            _scnt += _fsize;
	}
        process ();
    }

    _alsa_handle->pcm_stop ();
    put_event (EV_EXIT);
}


void Audio::init_jack (const char *server)
{
    char           s [16];
    int             opts;
    jack_status_t  stat;

    opts = JackNoStartServer;
    if (server) opts |= JackServerName;
    if ((_jack_handle = jack_client_open (_jname, (jack_options_t) opts, &stat, server)) == 0)
    {
        fprintf (stderr, "Can't connect to JACK\n");
        exit (1);
    }

    jack_set_process_callback (_jack_handle, jack_static_callback, (void *)this);
    jack_on_shutdown (_jack_handle, jack_static_shutdown, (void *)this);

    if (jack_activate (_jack_handle))
    {
        fprintf(stderr, "Can't activate JACK");
        exit (1);
    }
    _run_jack = true;

    for (int i = 0; i < _nplay; i++)
    {
        sprintf(s, "out_%d", i + 1);
        _jack_out [i] = jack_port_register (_jack_handle, s, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    }
    for (int i = 0; i < _ncapt; i++)
    {
        sprintf(s, "in_%d", i + 1);
        _jack_in [i] = jack_port_register (_jack_handle, s, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    }

    _outs = new float [4096];
    init ();
    
    _fsamp = jack_get_sample_rate (_jack_handle);
    _fsize = jack_get_buffer_size (_jack_handle);
    _jname = jack_get_client_name (_jack_handle);
    _cmain->put_event (EV_MESG, new M_jinfo (_fsamp, _fsize, _jname));
    _active = true;
}


void Audio::close_jack ()
{
    jack_deactivate (_jack_handle);
    jack_client_close (_jack_handle);
}


void Audio::jack_static_shutdown (void *arg)
{
    return ((Audio *) arg)->jack_shutdown ();
}


void Audio::jack_shutdown (void)
{
    _cmain->put_event (EV_JACK);
}


int Audio::jack_static_callback (jack_nframes_t nframes, void *arg)
{
    return ((Audio *) arg)->jack_callback (nframes);
}


int Audio::jack_callback (jack_nframes_t nframes)
{
    unsigned long  b, m, n;
    int     i;
    float  *p;
 
    if (!_active) return 0;
    if (_data && _input >= 0)
    {
        p = (float *)(jack_port_get_buffer (_jack_in [_input], nframes));
	m = nframes;
        n = _size - _dind;
        if (m >= n)
	{
            memcpy (_data + _dind, p, sizeof(jack_default_audio_sample_t) * n);
            _dind = 0;
            p += n;
            m -= n;
        }
        if (m)
	{
            memcpy (_data + _dind, p, sizeof(jack_default_audio_sample_t) * m);
            _dind += m;
	}
        _scnt += nframes;
    }

    generate (nframes);
    b = _g_bits & 255;   
    for (i = 0; i < _nplay; i++, b >>= 1)
    {    
          
        p = (float *)(jack_port_get_buffer (_jack_out [i], nframes));
    	if (b & 1) memcpy (p, _outs, sizeof(jack_default_audio_sample_t) * nframes);
        else       memset (p, 0,     sizeof(jack_default_audio_sample_t) * nframes);
    }

    process ();

    return 0;
}


void Audio::process (void) 
{
    int       k;
    ITC_mesg *M;

    if (_data)
    {
        k = _scnt / _step;
        if (k && _cmain->put_event_try (EV_TRIG, k) == ITC_ctrl::NO_ERROR) _scnt -= k * _step;
    }
   
    if (get_event_nowait (1 << EV_MESG) == EV_MESG)
    {
	M = get_message ();
	if (M->type () == M_BUFFP)
	{
	    M_buffp *Z = (M_buffp *) M; 
	    _data = Z->_data;
	    _size = Z->_size;
	    _step = Z->_step; 
	    _dind = 0;
	    _scnt = 0;
	}
	else if (M->type () == M_INPUT)
	{
	    M_input *Z = (M_input *) M; 
	    _input = Z->_input;
	    if (_input >= _ncapt) _input = -1;
	}
	else if (M->type () == M_GENPAR)
	{
	    M_genpar *Z = (M_genpar *) M; 
            _g_bits  = Z->_g_bits;  
            _a_noise = sqrt (0.5) * pow (10.0, 0.05 * Z->_a_noise);
            _a_sine1  = pow (10.0, 0.05 * Z->_a_sine1);
            _f_sine1  = LSINE * Z->_f_sine1 / _fsamp; 
            _a_sine2  = pow (10.0, 0.05 * Z->_a_sine2);
            _f_sine2  = LSINE * Z->_f_sine2 / _fsamp; 
	}
	M->recover ();
    }
}


void Audio::generate (int size) 
{
    int   i, j;
    float a, p, r;

    if (size > 4096) size = 4096;

    memset (_outs, 0, size * sizeof (float));

    if (_g_bits & M_genpar::WNOISE)
    {
	for (i = 0; i < size; i++)
	{
	    _outs [i] += _a_noise * _rngen.grandf ();
	}
    }
    if (_g_bits & M_genpar::SINE1)
    {  
        p = _p_sine1;       
	for (i = 0; i < size; i++)
        {
	    j = (int) p;
            if (j == LSINE) a = 0;
	    else
	    {
   	        r = p - j;
                a = (1.0 - r) * _sine [j] + r * _sine [j + 1];
	    }
            _outs [i] += _a_sine1 * a;
            p += _f_sine1;
            if (p >= LSINE) p -= LSINE;         
	}
        _p_sine1 = p;       
    }
    if (_g_bits & M_genpar::SINE2)
    {  
        p = _p_sine2;       
	for (i = 0; i < size; i++)
        {
	    j = (int) p;
            if (j == LSINE) a = 0;
	    else
	    {
   	        r = p - j;
                a = (1.0 - r) * _sine [j] + r * _sine [j + 1];
	    }
            _outs [i] += _a_sine2 * a;
            p += _f_sine2;
            if (p >= LSINE) p -= LSINE;         
	}
        _p_sine2 = p;       
    }
}
