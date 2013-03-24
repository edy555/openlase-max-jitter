/* 
	Copyright 2013
	TT edy555@gmail.com
*/

#include "jit.common.h"
#include "ext_systhread.h"
#import "libol.h"
#import "trace.h"

#define FRAMES_BUF 8

typedef struct _jit_openlase_trace 
{
	t_object	ob;
	int			initialized;
	OLTraceCtx *trace_ctx;
	OLRenderParams params;
	OLTraceParams tparams;
	float overscan;
	float aspect;
	int decimate;
	int snap_pix;
	long		planecount;
	t_systhread_mutex mutex;
} t_jit_openlase_trace;

t_jit_err jit_openlase_trace_init(void); 
t_jit_err jit_openlase_trace_matrix_calc(t_jit_openlase_trace *x, void *inputs, void *outputs);

void *_jit_openlase_trace_class;

t_jit_openlase_trace *jit_openlase_trace_new(void);
void jit_openlase_trace_free(t_jit_openlase_trace *x);

void openlase_initialize(t_jit_openlase_trace *x, int width, int height);
void openlase_trace(t_jit_openlase_trace *x, int width, int height, uint8_t *base, unsigned bytesperrow);


t_jit_err jit_openlase_trace_init(void) 
{
	long attrflags=0;
	t_jit_object *attr, *mop;
	
	_jit_openlase_trace_class = jit_class_new("jit_openlase_trace",(method)jit_openlase_trace_new,(method)jit_openlase_trace_free,
		sizeof(t_jit_openlase_trace),0L);

	//add mop
	mop = jit_object_new(_jit_sym_jit_mop,1,0);
	jit_class_addadornment(_jit_openlase_trace_class, mop);
	//add methods
	jit_class_addmethod(_jit_openlase_trace_class, (method)jit_openlase_trace_matrix_calc, 		"matrix_calc", 		A_CANT, 0L);

	//add attributes	
	attrflags = JIT_ATTR_GET_DEFER_LOW | JIT_ATTR_SET_USURP_LOW;	
	CLASS_STICKY_ATTR(_jit_openlase_trace_class,"category",0,"Laser");
	CLASS_STICKY_ATTR(_jit_openlase_trace_class,"basic",0,"1");
	
	attr = jit_object_new(_jit_sym_jit_attr_offset, "rate", _jit_sym_long, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, params.rate));
	jit_class_addattr(_jit_openlase_trace_class,attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "rate", 0, "rate: Speed of Laser Scanner (Hz)");

	attr = jit_object_new(_jit_sym_jit_attr_offset, "on_speed", _jit_sym_float32, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, params.on_speed));
	jit_class_addattr(_jit_openlase_trace_class,attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "on_speed", 0, "");
	
	attr = jit_object_new(_jit_sym_jit_attr_offset, "off_speed", _jit_sym_float32, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, params.off_speed));
	jit_class_addattr(_jit_openlase_trace_class,attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "off_speed", 0, "");
	
	attr = jit_object_new(_jit_sym_jit_attr_offset, "start_wait", _jit_sym_long, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, params.start_wait));
	jit_class_addattr(_jit_openlase_trace_class,attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "start_wait", 0, "");
	
	attr = jit_object_new(_jit_sym_jit_attr_offset, "end_wait", _jit_sym_long, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, params.end_wait));
	jit_class_addattr(_jit_openlase_trace_class, attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "end_wait", 0, "");
	
	attr = jit_object_new(_jit_sym_jit_attr_offset, "start_dwell", _jit_sym_long, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, params.start_dwell));
	jit_class_addattr(_jit_openlase_trace_class, attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "start_dwell", 0, "");
	
	attr = jit_object_new(_jit_sym_jit_attr_offset, "end_dwell", _jit_sym_long, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, params.end_dwell));
	jit_class_addattr(_jit_openlase_trace_class, attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "end_dwell", 0, "");
	
	attr = jit_object_new(_jit_sym_jit_attr_offset, "corner_dwell", _jit_sym_long, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, params.corner_dwell));
	jit_class_addattr(_jit_openlase_trace_class, attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "corner_dwell", 0, "");
	
	attr = jit_object_new(_jit_sym_jit_attr_offset, "min_length", _jit_sym_long, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, params.min_length));
	jit_class_addattr(_jit_openlase_trace_class, attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "min_length", 0, "");

	attr = jit_object_new(_jit_sym_jit_attr_offset, "overscan", _jit_sym_float32, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, overscan));
	jit_class_addattr(_jit_openlase_trace_class, attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "overscan", 0, "");
	
	attr = jit_object_new(_jit_sym_jit_attr_offset, "aspect", _jit_sym_float32, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, aspect));
	jit_class_addattr(_jit_openlase_trace_class, attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "aspect", 0, "");
	
	attr = jit_object_new(_jit_sym_jit_attr_offset, "decimate", _jit_sym_long, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, decimate));
	jit_class_addattr(_jit_openlase_trace_class, attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "decimate", 0, "");
	
	attr = jit_object_new(_jit_sym_jit_attr_offset, "snap_pix", _jit_sym_long, attrflags,
						  (method)0L, (method)0L, calcoffset(t_jit_openlase_trace, snap_pix));
	jit_class_addattr(_jit_openlase_trace_class, attr);
	CLASS_ATTR_LABEL(_jit_openlase_trace_class, "snap_pix", 0, "");
	
	CLASS_STICKY_ATTR_CLEAR(_jit_openlase_trace_class, "category");
	CLASS_STICKY_ATTR_CLEAR(_jit_openlase_trace_class, "basic");
	
	jit_class_register(_jit_openlase_trace_class);
	return JIT_ERR_NONE;
}

t_jit_err jit_openlase_trace_matrix_calc(t_jit_openlase_trace *x, void *inputs, void *outputs)
{
	t_jit_err err=JIT_ERR_NONE;
	long in_savelock;
	t_jit_matrix_info in_minfo;
	char *in_bp;
	long dimcount;
	void *in_matrix;
	int width, height;
	unsigned bytesperrow;
	
	in_matrix 	= jit_object_method(inputs,_jit_sym_getindex,0);

	if (x&&in_matrix) {
		
		in_savelock = (long) jit_object_method(in_matrix,_jit_sym_lock,1);
		jit_object_method(in_matrix,_jit_sym_getinfo,&in_minfo);
		jit_object_method(in_matrix,_jit_sym_getdata,&in_bp);
		
		if (!in_bp) { err=JIT_ERR_INVALID_INPUT; 	x->planecount = 0; goto out;}
		
		//get dimensions/planecount 
		dimcount    = in_minfo.dimcount;
		if (dimcount != 2) {
			object_post((t_object *)x, "requires matrix dimension equals to 2");
			err=JIT_ERR_INVALID_INPUT; 	goto out;
		}
		width = in_minfo.dim[0];
		height = in_minfo.dim[1];
		bytesperrow = in_minfo.dimstride[1];
		
		// check matrix type
		if (in_minfo.type != _jit_sym_char) {
			object_post((t_object *)x, "requires matrix type is char");
			err=JIT_ERR_INVALID_INPUT; goto out;
		}
		if (in_minfo.planecount != 1) {
			object_post((t_object *)x, "requires matrix has just one plane");
			err=JIT_ERR_INVALID_INPUT; goto out;
		}
		
		openlase_initialize(x, width, height);
		openlase_trace(x, width, height, (uint8_t*)in_bp, bytesperrow);
	} else {
		return JIT_ERR_INVALID_PTR;
	}
	
out:
	jit_object_method(in_matrix,_jit_sym_lock,in_savelock);
	return err;
}

t_jit_openlase_trace *jit_openlase_trace_new(void)
{
	t_jit_openlase_trace *x;
		
	if (x=(t_jit_openlase_trace *)jit_object_alloc(_jit_openlase_trace_class)) {
		x->planecount = 0;
				
		x->params.rate = 12000;
		x->params.on_speed = 2.0/100.0;
		x->params.off_speed = 2.0/20.0;
		x->params.snap = 1/120.0;
		x->params.render_flags = RENDER_GRAYSCALE;
		x->params.min_length = 20;
		x->params.start_wait = 15;
		x->params.end_wait = 3;
		x->params.start_dwell = 10;
		x->params.end_dwell = 10;
		x->params.corner_dwell = 12;
		x->overscan = 0.0;
		x->aspect = 0.0;
		x->decimate = 1;
		x->snap_pix = 3;
		
		systhread_mutex_new(&x->mutex,0);
	} else {
		x = NULL;
	}
	return x;
}

void jit_openlase_trace_free(t_jit_openlase_trace *x)
{
	if (x->trace_ctx != NULL)
		olTraceDeinit(x->trace_ctx);
	x->trace_ctx = NULL;
	
	olShutdown();
	
	systhread_mutex_free(x->mutex);
}

void openlase_initialize(t_jit_openlase_trace *x, int width, int height)
{
	if (x->initialized) {
		return;
	}

	//object_post((t_object *)x, "openlase_initialize enter");
	
	if(olInit(FRAMES_BUF, 300000) < 0) {
		object_post((t_object *)x, "failed to initialize OpenLase");
		return;
	}
	
	float overscan = x->overscan;
	float aspect = x->aspect;
	float snap_pix = x->snap_pix;
	//float framerate = 30;
	
	x->tparams.mode = OL_TRACE_THRESHOLD;
	x->tparams.sigma = 0;
	x->tparams.threshold2 = 50;
	
	//x->tparams.mode = OL_TRACE_CANNY;
	x->tparams.sigma = 1;
	
	if (aspect == 0)
		aspect = (float)width / height;
	
	//	if (framerate == 0)
	//		framerate = (float)pFormatCtx->streams[videoStream]->r_frame_rate.num / (float)pFormatCtx->streams[videoStream]->r_frame_rate.den;
	
	float iaspect = 1/aspect;
	
	if (aspect > 1) {
		olSetScissor(-1, -iaspect, 1, iaspect);
		olScale(1, iaspect);
	} else {
		olSetScissor(-aspect, -1, aspect, 1);
		olScale(aspect, 1);
	}
	
	olScale(1+overscan, 1+overscan);
	olTranslate(-1.0f, 1.0f);
	olScale(2.0f/width, -2.0f/height);
	
	int maxd = width > height ? width : height;
	x->params.snap = (snap_pix*2.0)/(float)maxd;
	
	//float frametime = 1.0f/framerate;
	
	olSetRenderParams(&x->params);
	
	x->tparams.width = width,
	x->tparams.height = height,
	olTraceInit(&x->trace_ctx, &x->tparams);
	
	//object_post((t_object *)x, "openlase initialized");
	
	x->initialized = true;
}

void
openlase_trace(t_jit_openlase_trace *x, int width, int height, uint8_t *base, unsigned bytesperrow)
{
	//float vidtime = 0;
	//int inf=0;
	int bg_white = -1;
	//float time = 0;
	float ftime;
	//int frames = 0;
	
	OLTraceResult result;	
	memset(&result, 0, sizeof(result));
	
	int thresh;
	int obj;
	int bsum = 0;
	int c;

	int thresh_dark;
	int thresh_light;
	int sw_dark;
	int sw_light;
	int decimate = x->decimate;
	int edge_off;
	thresh_dark = 60;
	thresh_light = 160;
	sw_dark = 100;
	sw_light = 256;
	//decimate = 1;
	//decimate = 2;
	edge_off = 0;
	
	for (c=edge_off; c<(width-edge_off); c++) {
		bsum += base[c+edge_off*bytesperrow];
		bsum += base[c+(height-edge_off-1)*bytesperrow];
	}
	for (c=edge_off; c<(height-edge_off); c++) {
		bsum += base[edge_off+c*bytesperrow];
		bsum += base[(c+1)*bytesperrow-1-edge_off];
	}
	bsum /= (2*(width+height));
	if (bg_white == -1)
		bg_white = bsum > 128;
	if (bg_white && bsum < sw_dark)
		bg_white = 0;
	if (!bg_white && bsum > sw_light)
		bg_white = 1;
	
	if (bg_white)
		thresh = thresh_light;
	else
		thresh = thresh_dark;
	
	x->tparams.threshold = thresh;
	olTraceReInit(x->trace_ctx, &x->tparams);
	olTraceFree(&result);
	obj = olTrace(x->trace_ctx, base, bytesperrow, &result);
	
	//do {
		int i, j;
		for (i = 0; i < result.count; i++) {
			OLTraceObject *o = &result.objects[i];
			olBegin(OL_POINTS);
			OLTracePoint *p = o->points;
			for (j = 0; j < o->count; j++) {
				if (j % decimate == 0)
					olVertex(p->x, p->y, C_WHITE);
				p++;
			}
			olEnd();
		}
		
		ftime = olRenderFrame(200);
	
	if (0) {
		OLFrameInfo info;	
		char msg[256];
		olGetFrameInfo(&info);
		sprintf(msg, "%d:%d Thr %3d Bg %3d Pts %4d", width, height, thresh, bsum, info.points);	
		object_post((t_object *)x, msg);
	}
		//frames++;
		//time += ftime;
		
		//printf("Frame time: %.04f, Cur FPS:%6.02f, Avg FPS:%6.02f, Drift: %7.4f, "
		//	   "In %4d, Out %4d Thr %3d Bg %3d Pts %4d",
		//	   ftime, 1/ftime, frames/time, time-vidtime,
		//	   inf, frames, thresh, bsum, info.points);
		//if (info.resampled_points)
		//	printf(" Rp %4d Bp %4d", info.resampled_points, info.resampled_blacks);
		//if (info.padding_points)
		//	printf(" Pad %4d", info.padding_points);
		//printf("\n");
	//} while ((time+frametime) < vidtime);

	olTraceFree(&result);
}	
