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

#include "avframe.h"
#include <QImage>
#include <cstdlib>
#include <cstdio>
#include "types.h"

avframe::avframe() : tobefreed(0),w(0),h(0),dw(0),pix_fmt(),img_convert_ctx(0)
{
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(55, 19, 0)
    f=av_frame_alloc();
#else
    f=avcodec_alloc_frame();
#endif
}

avframe::avframe(AVFrame *src, AVCodecContext *ctx) : f(0),tobefreed(0)
{
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(55, 19, 0)
    f=av_frame_alloc();
#else
    f=avcodec_alloc_frame();
#endif

    tobefreed = (void*) calloc(1, avpicture_get_size(ctx->pix_fmt, ctx->width, ctx->height));

    avpicture_fill((AVPicture *)f,
                   (u_int8_t*)tobefreed,
                   ctx->pix_fmt,ctx->width,ctx->height);

#if LIBAVCODEC_VERSION_INT >= (51 << 16)
    av_picture_copy((AVPicture *)f, (const AVPicture *) src,
                    ctx->pix_fmt, ctx->width, ctx->height);
#else
    img_copy((AVPicture *)f, (const AVPicture *) src,
             ctx->pix_fmt, ctx->width, ctx->height);
#endif

    f->pict_type              = src->pict_type;
    f->quality                = src->quality;
    f->coded_picture_number   = src->coded_picture_number;
    f->display_picture_number = src->display_picture_number;
    f->pts                    = src->pts;
    f->interlaced_frame       = src->interlaced_frame;
    f->top_field_first        = src->top_field_first;
    f->repeat_pict            = src->repeat_pict;
    f->quality                = src->quality;

    w=ctx->width;
    h=ctx->height;
    pix_fmt=ctx->pix_fmt;
    dw=w*ctx->sample_aspect_ratio.num/ctx->sample_aspect_ratio.den;
    img_convert_ctx=sws_getContext(w, h, pix_fmt,
                                   w, h, PIX_FMT_BGR24, SWS_BICUBIC,
                                   NULL, NULL, NULL);
}

avframe::~avframe()
{
    if (tobefreed)
        free(tobefreed);
    if (f)
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(55, 19, 0)
        av_frame_free(&f);
#else
        av_free(f);
#endif

    if (img_convert_ctx)
        sws_freeContext(img_convert_ctx);
}

QImage avframe::getqimage(bool scaled, double viewscalefactor)
{
    if (w<=0 || h<=0 || img_convert_ctx==NULL)
        return QImage();

    uint8_t *rgbbuffer = (uint8_t*) calloc(1, avpicture_get_size(PIX_FMT_RGB24, w, h)+64);
    int headerlen=sprintf((char *) rgbbuffer, "P6\n%d %d\n255\n", w, h);

#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(55, 19, 0)
    AVFrame *avframergb = av_frame_alloc();
#else
    AVFrame *avframergb = avcodec_alloc_frame();
#endif

    avpicture_fill((AVPicture*)avframergb,
                   rgbbuffer+headerlen,
                   PIX_FMT_RGB24,w,h);

    sws_scale(img_convert_ctx, f->data, f->linesize, 0, h, avframergb->data, avframergb->linesize);

    QImage im;
    im.loadFromData(rgbbuffer, headerlen+w*h*3, "PPM");

    im = im.rgbSwapped();

    if ((scaled && w!=dw)||(viewscalefactor!=1.0)) {
#ifdef SMOOTHSCALE
        im = im.scaled(static_cast<int>((scaled?dw:w)/viewscalefactor + 0.5), static_cast<int>(h/viewscalefactor + 0.5), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
#else
        im = im.scaled(static_cast<int>((scaled?dw:w)/viewscalefactor + 0.5), static_cast<int>(h/viewscalefactor + 0.5));
#endif
    }

    free(rgbbuffer);

#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(55, 19, 0)
    av_frame_free(&avframergb);
#else
    av_free(avframergb);
#endif

    return (im);
}
