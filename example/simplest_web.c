#include "emn.h"
#include <stdio.h>

static void signal_cb(struct ev_loop *loop, ev_signal *w, int revents)
{
	printf("Got signal: %d\n", w->signum);
	ev_break(loop, EVBREAK_ALL);
}

void event_handler(struct emn_client *cli, int event, void *data)
{
	switch (event) {
    case EMN_EV_ACCEPT: {
			struct sockaddr_in *sin = (struct sockaddr_in *)data;
	        printf("%p: new conn from %s:%d\n", cli, inet_ntoa(sin->sin_addr), ntohs(sin->sin_port));
        	break;
    	}
	case EMN_EV_RECV: {
#if 0
			struct ebuf *rbuf = emn_get_rbuf(cli);
			int len = *(int *)data;
			printf("recv %d: [%.*s]\n", len, (int)rbuf->len, rbuf->buf);
			//ebuf_remove(rbuf, len);
#endif			
			break;
		}
	case EMN_EV_HTTP_REQUEST: {
			enum http_method method = emn_get_http_method(cli);
			struct emn_str *uri = emn_get_http_uri(cli);
			struct emn_str *host = emn_get_http_header(cli, "host");
			struct emn_str *body = emn_get_http_body(cli);
			
			printf("method: %s\n", http_method_str(method));
			printf("uri: %.*s\n", (int)uri->len, uri->p);
			printf("proto: %d.%d\n", emn_get_http_version_major(cli), emn_get_http_version_minor(cli));
			printf("Host: %.*s\n", (int)host->len, host->p);
			printf("body: %.*s\n", (int)body->len, body->p);

			emn_serve_http(cli);
			break;
		}
	case EMN_EV_CLOSE: {
			printf("client(%p) closed\n", cli);
			break;
		}
    default:
		break;
    }
}

int main(int argc, char **argv)
{
	struct ev_loop *loop = EV_DEFAULT;
	ev_signal sig_watcher;
	struct emn_server *srv = NULL;
	const char *address = "8000";
	
	printf("emn version: %d.%d\n", EMN_VERSION_MAJOR, EMN_VERSION_MINOR);
	
	ev_signal_init(&sig_watcher, signal_cb, SIGINT);
	ev_signal_start(loop, &sig_watcher);
	
	srv = emn_bind(loop, address, event_handler);
	if (!srv) {
		printf("emn_bind failed\n");
		goto err;
	}
	
	emn_set_protocol_http(srv);
	printf("%p: listen on: %s\n", srv, address);
	
	ev_run(loop, 0);

err:	
	emn_server_destroy(srv);
	printf("Server exit...\n");
		
	return 0;
}
