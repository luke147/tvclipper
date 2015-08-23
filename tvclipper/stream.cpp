/*  tvclipper
    Copyright (c) 2005 Sven Over <svenover@svenover.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* $Id: stream.cpp 97 2015-08-20 19:13:12Z too-tired $ */

#include "stream.h"


stream::~stream()
{
    if (avcc)
        av_free(avcc);
}

void stream::freeavcc()
{
    if (avcc)
        av_free(avcc);
    avcc=0;
}

void stream::allocavcc()
{
    if (avcc)
        av_free(avcc);
    // this guy needs a parameter of..?  dec ?
    //AVCodecContext * 	avcodec_alloc_context3 (const AVCodec *codec)
    // 	Allocate an AVCodecContext and set its fields to default values.
    avcc=avcodec_alloc_context3(dec);

    //int 	avcodec_get_context_defaults3 (AVCodecContext *s, const AVCodec *codec)
    // 	Set the fields of the given AVCodecContext to default values corresponding to the given
    // codec (defaults may be codec-dependent).
    avcodec_get_context_defaults3(avcc, dec);
}

void stream::setvideoencodingparameters(bool interlaced)
{
    avcc->bit_rate=9500000;
    avcc->rc_min_rate=9500000;
    avcc->rc_max_rate=9500000;
    avcc->rc_buffer_size=224*1024*8;
#warning zakomentování zastaralého atributu AVCodecContext::rc_buffer_aggressivity: function avcodec_find_encoder returns AVCodec struct pointer;\
     AVCodec struct contains AVClass struct; AVClass struct contains AVOption struct which is contant; see reference AVCodecContext::rc_buffer_aggressivity
    // avcc->rc_buffer_aggressivity=1.0;
    avcc->rc_initial_buffer_occupancy = avcc->rc_buffer_size*3/4;
    avcc->qmax=2;
    avcc->mb_lmax= FF_QP2LAMBDA * 2;
#warning zakomentování zastaralého atributu AVCodecContext::lmax: function avcodec_find_encoder returns AVCodec struct pointer;\
     AVCodec struct contains AVClass struct; AVClass struct contains AVOption struct which is contant; see reference AVCodecContext::lmax
    // avcc->lmax= FF_QP2LAMBDA * 2;
    if (interlaced)
        avcc->flags |= CODEC_FLAG_INTERLACED_DCT|CODEC_FLAG_INTERLACED_ME;
}

const std::string& stream::getinfo() const
{
    return infostring;
}
