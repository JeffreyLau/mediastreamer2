/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "mediastreamer2/msfilter.h"
#include <bv16/typedef.h>
#include <bv16/bv16cnst.h>
#include <bv16/bvcommon.h>
#include <bv16/bv16strct.h>
#include <bv16/bv16.h>


#ifdef _MSC_VER
#include <malloc.h>
#define alloca _alloca
#endif

typedef struct EncState{
	struct BV16_Encoder_State state;
	uint32_t ts; // timestamp
	int ptime;
	int maxptime;
	MSBufferizer *bufferizer;
} EncState;

static int set_ptime(MSFilter *f, int ptime){
	 EncState *s=(EncState*)f->data;
	 if (ptime<10 || ptime>140) return -1;
	 s->ptime = ptime;
	 ms_message("MSBV16Enc: got ptime=%i using [%i]",ptime,s->ptime);
	return 0;
}

static int enc_add_fmtp(MSFilter *f, void *arg){
	 const char *fmtp=(const char *)arg;
	 char tmp[30];
	 if (fmtp_get_value(fmtp,"ptime",tmp,sizeof(tmp))){
	 	return set_ptime(f,atoi(tmp));
	 }
	return 0;
}

static int enc_add_attr(MSFilter *f, void *arg){
	 const char *attr=(const char *)arg;
	 if (strstr(attr,"ptime:")!=NULL){
	 	int ptime = atoi(attr+6);
	 	return set_ptime(f,ptime);
	 }
	return 0;
}

static int enc_get_sample_rate(MSFilter *f, void *arg) {
	MS_UNUSED(f);
	*((int *)arg) = 8000;
	return 0;
}

static int get_channels(MSFilter *f, void *arg) {
	*((int *)arg) = 1;
	return 0;
}
static void enc_init(MSFilter *f){
	EncState *s=(EncState *)ms_new(EncState,1);
	Reset_BV16_Encoder(&(s->state));
	s->ts=0;
	s->ptime=0;
	s->maxptime = 0;
	s->bufferizer=ms_bufferizer_new();
	f->data=s;
}

static void enc_uninit(MSFilter *f){
	 EncState *s=(EncState*)f->data;
	 ms_bufferizer_destroy(s->bufferizer);
	 ms_free(s);
}



static void enc_process(MSFilter *f){
//	 EncState *s=(EncState*)f->data;
//	 mblk_t *im;
//	 unsigned int unitary_buff_size = sizeof(int16_t)*160;
//	 unsigned int buff_size = unitary_buff_size*s->ptime/20;
//	 int16_t* buff;
//	 unsigned int offset;
//	
//	 while((im=ms_queue_get(f->inputs[0]))!=NULL){
//	 	ms_bufferizer_put(s->bufferizer,im);
//	 }
//	 while(ms_bufferizer_get_avail(s->bufferizer) >= buff_size) {
//	 	mblk_t *om=allocb((33*s->ptime)/20,0);
//	 	buff = (int16_t *)alloca(buff_size);
//	 	ms_bufferizer_read(s->bufferizer,(uint8_t*)buff,buff_size);
//		
//	 	for (offset=0;offset<buff_size;offset+=unitary_buff_size) {
//	 		bv16_encode(s->state,(bv16_signal*)&buff[offset/sizeof(int16_t)],(bv16_byte*)om->b_wptr);
//	 		om->b_wptr+=33;
//	 	}
//	 	ms_bufferizer_fill_current_metas(s->bufferizer, om);
//	 	mblk_set_timestamp_info(om,s->ts);
//	 	ms_queue_put(f->outputs[0],om);
//	 	s->ts+=buff_size/sizeof(int16_t)/*sizeof(buf)/2*/;

}

static MSFilterMethod enc_methods[]={
	{MS_FILTER_ADD_FMTP			,enc_add_fmtp},
	{MS_FILTER_ADD_ATTR        	,enc_add_attr},
	{MS_FILTER_GET_SAMPLE_RATE	,enc_get_sample_rate },
	{	0				,	NULL		}
};

#ifdef _MSC_VER

MSFilterDesc ms_bv16_enc_desc={
	MS_BV16_ENC_ID,
	"MSBv16Enc",
	N_("The BV16 full-rate codec"),
	MS_FILTER_ENCODER,
	"bv16",
	1,
	1,
	enc_init,
	NULL,
	enc_process,
	NULL,
	enc_uninit,
	enc_methods
};

#else

MSFilterDesc ms_bv16_enc_desc={
	.id=MS_BV16_ENC_ID,
	.name="MSBv16Enc",
	.text=N_("The BV16 full-rate codec"),
	.category=MS_FILTER_ENCODER,
	.enc_fmt="bv16",
	.ninputs=1,
	.noutputs=1,
	.init=enc_init,
	.process=enc_process,
	.uninit=enc_uninit,
	.methods = enc_methods
};

#endif

static void dec_init(MSFilter *f){
	Reset_BV16_Decoder((struct BV16_Decoder_State *)f->data);
}

static void dec_uninit(MSFilter *f){
	Reset_BV16_Decoder((struct BV16_Decoder_State *)f->data);

}


static void dec_process(MSFilter *f){
	// gsm s=(gsm)f->data;
	// mblk_t *im;
	// mblk_t *om;
	// const int frsz=160*2;

	// while((im=ms_queue_get(f->inputs[0]))!=NULL){
	// 	for (;(im->b_wptr-im->b_rptr)>=33;im->b_rptr+=33) {
	// 		om=allocb(frsz,0);
	// 		mblk_meta_copy(im, om);
	// 		if (gsm_decode(s,(gsm_byte*)im->b_rptr,(gsm_signal*)om->b_wptr)<0){
	// 			ms_warning("gsm_decode error!");
	// 			freemsg(om);
	// 		}else{
	// 			om->b_wptr+=frsz;
	// 			ms_queue_put(f->outputs[0],om);
	// 		}
	// 	}
	// 	freemsg(im);
	// }
}

static int dec_get_sample_rate(MSFilter *f, void *arg) {
	MS_UNUSED(f);
	*((int *)arg) = 8000;
	return 0;
}

static MSFilterMethod dec_methods[] = {
	{ MS_FILTER_GET_SAMPLE_RATE, dec_get_sample_rate },
	{ 0,                         NULL                }
};

#ifdef _MSC_VER

MSFilterDesc ms_bv16_dec_desc={
	MS_BV16_DEC_ID,
	"MSBv16Dec",
	N_("The BV16 codec"),
	MS_FILTER_DECODER,
	"bv16",
	1,
	1,
	dec_init,
	NULL,
	dec_process,
	NULL,
	dec_uninit,
	dec_methods
};

#else

MSFilterDesc ms_bv16_dec_desc={
	.id=MS_BV16_DEC_ID,
	.name="MSBv16Dec",
	.text=N_("The BV16 codec"),
	.category=MS_FILTER_DECODER,
	.enc_fmt="bv16",
	.ninputs=1,
	.noutputs=1,
	.init=dec_init,
	.process=dec_process,
	.uninit=dec_uninit,
	.methods=dec_methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_bv16_dec_desc)
MS_FILTER_DESC_EXPORT(ms_bv16_enc_desc)
