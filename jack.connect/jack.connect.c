/**
	@file
	jack.connect - connect and disconnect routing on jack
*/

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object
#include <jack/jack.h>	// located /usr/local/include


////////////////////////// object struct
typedef struct _jackconnect 
{
	t_object ob;			// the object itself (must be first)
	jack_client_t* client;
} t_jackconnect;

///////////////////////// function prototypes
//// standard set
void *jackconnect_new(t_symbol *s, long argc, t_atom *argv);
void jackconnect_free(t_jackconnect *x);
void jackconnect_connect(t_jackconnect *x, t_symbol *p1, t_symbol *p2);
void jackconnect_disconnect(t_jackconnect *x, t_symbol *p1, t_symbol *p2);
//////////////////////// global class pointer variable
void *jackconnect_class;

static char *my_name = "jack.connect";


int main(void)
{	
	t_class *c;
	
	c = class_new("jack.connect", (method)jackconnect_new, (method)jackconnect_free, (long)sizeof(t_jackconnect), 
				  0L, A_GIMME, 0);
	
    class_addmethod(c, (method)jackconnect_connect,			"connect",		A_DEFSYM, A_DEFSYM, 0);  
    class_addmethod(c, (method)jackconnect_disconnect,		"disconnect",	A_DEFSYM, A_DEFSYM, 0);  
 	
	class_register(CLASS_BOX, c);
	jackconnect_class = c;

	return 0;
}

void jackconnect_free(t_jackconnect *x)
{
	if (x->client != NULL) {
		jack_deactivate (x->client);
		jack_client_close (x->client);
		x->client = NULL;
	}
}

void jackconnect_connect(t_jackconnect *x, t_symbol *p1, t_symbol *p2)
{
	jack_port_t *input_port;
	jack_port_t *output_port;

	if (x->client == NULL) {
		x->client = jack_client_open (my_name, JackNullOption, NULL);
		if (x->client == NULL) {
			object_post((t_object *)x, "jack server not running?");
			goto error;
		}
	}
	
	if ((input_port = jack_port_by_name(x->client, p1->s_name)) == 0) {
		object_post((t_object *)x, "not a valid port");
		goto error;
	}
	if ((output_port = jack_port_by_name(x->client, p2->s_name)) == 0) {
		object_post((t_object *)x, "not a valid port");
		goto error;
	}
	
	if (jack_connect(x->client, jack_port_name(input_port), jack_port_name(output_port))) {
		object_post((t_object *)x, "cannot connect ports");
		goto error;
	}

error:
	;
}

void jackconnect_disconnect(t_jackconnect *x, t_symbol *p1, t_symbol *p2)
{
	jack_port_t *input_port;
	jack_port_t *output_port;
	
	if (x->client == NULL) {
		x->client = jack_client_open (my_name, JackNullOption, NULL);
		if (x->client == NULL) {
			object_post((t_object *)x, "jack server not running?");
			goto error;
		}
	}
	
	if ((input_port = jack_port_by_name(x->client, p1->s_name)) == 0) {
		object_post((t_object *)x, "not a valid port");
		goto error;
	}
	if ((output_port = jack_port_by_name(x->client, p2->s_name)) == 0) {
		object_post((t_object *)x, "not a valid port");
		goto error;
	}
	
	if (jack_disconnect(x->client, jack_port_name(input_port), jack_port_name(output_port))) {
		object_post((t_object *)x, "cannot disconnect ports");
		goto error;
	}
	
error:
	;
}

void *jackconnect_new(t_symbol *s, long argc, t_atom *argv)
{
	t_jackconnect *x = NULL;
    
	x = (t_jackconnect *)object_alloc(jackconnect_class);
	x->client = NULL;
	return x;
}
