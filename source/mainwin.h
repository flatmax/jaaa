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


#ifndef __MAINWIN_H
#define __MAINWIN_H


#include <clxclient.h>
#include <clthreads.h>
#include <fftw3.h>
#include "styles.h"


#define XPOS  100
#define YPOS  100
#define XMIN  600
#define YMIN  430
#define XDEF  600
#define YDEF  450
#define XMAX  1200
#define YMAX  900
#define LMAR  40
#define RMAR  80
#define TMAR  8
#define BMAR  24


class Spectdata 
{
public:

    enum { MK1_SET = 1, MK1_NSE = 2, MK2_SET = 4, MK2_NSE = 8, RESET = 16, PEAKH = 32, FREEZE = 64, YP_VAL = 256, YM_VAL = 512 };

    Spectdata (int size)
    {
        _yp = new float [size];
        _ym = new float [size];
    }
    ~Spectdata (void)
    {
        delete[] _yp;
        delete[] _ym;
    }

    int    _npix;
    int    _bits;
    int    _avcnt;
    int    _avmax;
    float  _bw;
    float  _f0;
    float  _f1;
    float  _mk1f;
    float  _mk1p;
    float  _mk2f;
    float  _mk2p;
    char  *_xf;
    float *_yp;        
    float *_ym;
};


class Mainwin : public X_window, public X_callback
{
public:

    Mainwin (X_window *parent, X_resman *xres, ITC_ctrl *audio);
    ~Mainwin (void);
    bool running (void) const { return _running; }
    void handle_trig (void);
    void handle_term (void) { _running = 0; }
    void handle_mesg (ITC_mesg *);
 
private:

    enum
    { 
        IP1, IP2, IP3, IP4,
        IP5, IP6, IP7, IP8,
        BANDW, VIDAV, PEAKH, FREEZ,
        MCLR, MPEAK, MNSE,
        FMIN, FMAX, FCENT, FSPAN,
        AMAX, ASPAN,
        BTUP, BTDN,
        OP1, OP2, OP3, OP4,
        OP5, OP6, OP7, OP8,
        NSE_ACT, SI1_ACT, SI2_ACT,
        NSE_LEV, SI1_LEV, SI1_FREQ, SI2_LEV, SI2_FREQ,
        NBUTT
    };

    enum 
    {
        FFT_MIN = 256,
        FFT_MAX = 1024 * 256,
        BUF_LEN = 2 * FFT_MAX,
        INP_MAX = BUF_LEN - FFT_MAX / 2,
        INP_LEN = 4096
    };

    virtual void handle_event (XEvent *xe);
    virtual void handle_callb (int, X_window*, _XEvent*);

    void message (XClientMessageEvent *);
    void expose (XExposeEvent *);
    void resize (XConfigureEvent *);
    void redraw (void);
    void update (void);
    void bpress (XButtonEvent *);
    void motion (XPointerMovedEvent *);
    void brelse (XButtonEvent *);

    void set_fsamp (float fsamp, bool symm);
    void set_input (int i);
    void set_output (int i);
    void set_param (int i);
    void mod_param (bool inc);
    void set_bw (float);
    void set_f0 (float);
    void set_f1 (float);
    void set_fc (float);
    void set_fs (float);
    void set_a1 (float);
    void set_ar (float);
    void set_a_nse (float);
    void set_a_si1 (float);
    void set_f_si1 (float);
    void set_a_si2 (float);
    void set_f_si2 (float);
    void send_genp (void);
    void set_mark (float f, bool nse);
    void clr_mark (void);
    void show_param (void);
    void plot_fscale (void);
    void plot_ascale (void);
    void plot_clear (void);
    void plot_grid (void);
    void plot_spect (Spectdata *);
    void plot_annot (Spectdata *);
    void alloc_fft (Spectdata *);    
    void calc_spect (Spectdata *);
    float calcfreq (int x);
    float conv0 (fftwf_complex *);
    float conv1 (fftwf_complex *);
    void calc_noise (float *f, float *p);
    void calc_peak (float *f, float *p, float r);
    void print_note (char *s, float f);

    int         _xs, _ys;
    int         _running;
    Atom        _atoms [2];
    Atom        _wmpro;
    X_window   *_plotwin;
    Pixmap      _plotmap;
    GC          _plotgct;
    X_button   *_butt [NBUTT];
    X_textip   *_txt1;
    Spectdata  *_spect;
    ITC_ctrl   *_audio;

    int         _input;
    int         _drag;
    int         _xd, _yd;
    int         _p_ind;
    float       _p_val;
    float       _fsamp;
    float       _funit;
    const char *_fform;
    float       _bmin, _bmax, _bw;
    float       _fmin, _fmax, _f0, _f1, _fc, _fm, _df;
    float       _amin, _amax, _a0, _a1, _da;
    float       _a_nse, _a_si1, _f_si1, _a_si2, _f_si2;
    int         _g_bits;
    int         _ngx, _grx [40];
    int         _ngy, _gry [20];

    fftwf_complex  *_trbuf;
    fftwf_plan      _fftplan; 
    float          *_ipbuf;
    float          *_fftbuf;
    float          *_power;
    int             _fftlen;
    int             _ipmod;
    int             _ipcnt;
    float           _ptot;

    const static char *_formats [9];
    const static char *_notes [12];
};


#endif
