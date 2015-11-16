/*  dvbcut
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

 *  tvclipper
    Copyright (c) 2015 Lukáš Vlček

    This file is part of TV Clipper.

    TV Clipper is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TV Clipper is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TV Clipper. If not, see <http://www.gnu.org/licenses/>.
*/

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
#include <cstring>
#include <utility>
#include <list>
#include "avframe.h"
#include "streamhandle.h"
#include "lavfmuxer.h"

#include <stdio.h>

lavfmuxer::lavfmuxer(const char *format, uint32_t audiostreammask, mpgfile &mpg, const char *filename)
    : muxer(), avfc(0), fileopened(false)
{
    fmt = av_guess_format(format, NULL, NULL);
    if (!fmt) {
        return;
    }

    avfc=avformat_alloc_context();
    if (!avfc)
        return;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 35, 0)
    // todo: what here ?
    //  maybe: AVFormatContext::audio_preload but no direct access.
    //    AVOptions
    //    iformat
#else
    avfc->preload= (int)(.5*AV_TIME_BASE);
    avfc->mux_rate=10080000;
#endif
    avfc->max_delay= (int)(.7*AV_TIME_BASE);

    avfc->oformat=fmt;
    strncpy(avfc->filename, filename, sizeof(avfc->filename));

    int id=0;

    st[VIDEOSTREAM].stream_index=id;
    AVStream *s=st[VIDEOSTREAM].avstr=avformat_new_stream(avfc, NULL);
    s->id = id++;
    strpres[VIDEOSTREAM]=true;
    av_free(s->codec);
    mpg.setvideoencodingparameters();
    s->codec=mpg.getavcc(VIDEOSTREAM);
    s->codec->rc_buffer_size = 224*1024*8;
    s->sample_aspect_ratio = s->codec->sample_aspect_ratio;

    for (int i=0;i<mpg.getaudiostreams();++i)
        if (audiostreammask & (1u<<i)) {
            int astr=audiostream(i);
            st[astr].stream_index=id;
            s=st[astr].avstr=avformat_new_stream(avfc,NULL);
            s->id = id++;
            strpres[astr]=true;
            if (s->codec)
                av_free(s->codec);
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 35, 0)
            // todo: what here ?
            //  maybe need non NULL passed
            // AVCodecContext* avcodec_alloc_context3 	( 	const AVCodec *  	codec 	 )
            s->codec = avcodec_alloc_context3(NULL);

            // attribute_deprecated void avcodec_get_context_defaults 	( 	AVCodecContext *  	s 	 )
            avcodec_get_context_defaults3(s->codec, NULL);

#else
            s->codec = avcodec_alloc_context();
            avcodec_get_context_defaults(s->codec);
#endif
            s->codec->codec_type=AVMEDIA_TYPE_AUDIO;
            s->codec->codec_id = (mpg.getstreamtype(astr)==streamtype::ac3audio) ? AV_CODEC_ID_AC3 : AV_CODEC_ID_MP2;
            s->codec->rc_buffer_size = 224*1024*8;

            // Must read some packets to get codec parameters
            streamhandle sh(mpg.getinitialoffset());
            streamdata *sd=sh.newstream(astr,mpg.getstreamtype(astr),mpg.istransportstream());

            while (sh.fileposition < mpg.getinitialoffset()+(4<<20)) {
                if (mpg.streamreader(sh)<=0)
                    break;

                if (sd->getitemlistsize() > 1) {
                    // todo:
                    //int avcodec_open2 	( 	AVCodecContext *  	avctx,
                    //		const AVCodec *  	codec,
                    //		AVDictionary **  	options
                    //	)
                    if (!avcodec_open2(s->codec, avcodec_find_decoder(s->codec->codec_id), NULL)) {
                        AVFrame frame = AVFrame();
                        int got_frame_ptr;
                        s->codec->get_buffer2(s->codec, &frame,s->codec->flags);
                        //fprintf(stderr, "** decode audio size=%d\n", sd->inbytes());

                        AVPacket avpkt;
                        av_init_packet(&avpkt);
                        avpkt.data = (uint8_t*)sd->getdata();
                        avpkt.size = sd->inbytes();
                        // HACK for CorePNG to decode as normal PNG by default
                        avpkt.flags = AV_PKT_FLAG_KEY;
                        avcodec_decode_audio4(s->codec,&frame,&got_frame_ptr, &avpkt);
                        avcodec_close(s->codec);
                    }
                    break;
                }
            }
        }

    // error: 'av_set_parameters' was not declared in this scope
    if (!(fmt->flags & AVFMT_NOFILE)&&(avio_open(&avfc->pb, filename, AVIO_FLAG_WRITE) < 0)) {
        av_free(avfc);
        avfc=0;
        return;
    }
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 35, 0)
    // todo: what here ?
    //  maybe: AVFormatContext::audio_preload but no direct access.
    //    AVOptions
    //    iformat
#else
    avfc->preload= (int)(.5*AV_TIME_BASE);
    avfc->mux_rate=10080000;
#endif
    avfc->max_delay= (int)(.7*AV_TIME_BASE);

    av_dump_format(avfc, 0, filename, 1);
    fileopened=true;
    avformat_write_header(avfc, NULL);
}


lavfmuxer::~lavfmuxer()
{
    if (avfc) {
        if (fileopened) {
            av_write_trailer(avfc);
            if (!(fmt->flags & AVFMT_NOFILE))
                avio_close(avfc->pb);
        }

        av_free(avfc);
    }
}
