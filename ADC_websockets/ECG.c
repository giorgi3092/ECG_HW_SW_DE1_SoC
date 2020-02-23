#include "mongoose.h"           //  include Mongoose API definitions
#include "adc.h"                //  include function prototypes for ADC control

static sig_atomic_t s_signal_received = 0; 
static const char *s_http_port = "8080";
static struct mg_serve_http_opts s_http_server_opts;
unsigned int samples_taken = 0;

static const char *continuous_transmission_request = "start_continuous";
static const char *continuous_transmission_request_stop = "stop_continuous";

// store the ADC channel values here (global variable)
int channel_values[8];

static void signal_handler(int sig_num) {
  signal(sig_num, signal_handler);  // Reinstantiate signal handler
  s_signal_received = sig_num;
}

static int is_websocket(const struct mg_connection *nc) {
  return nc->flags & MG_F_IS_WEBSOCKET;
}

static void broadcast_options(struct mg_connection *nc, const struct mg_str msg) {
//  struct mg_connection *c;
  char buf[500];
  char addr[32];
  mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
                      MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

  snprintf(buf, sizeof(buf), "%s %.*s", addr, (int) msg.len, msg.p);
  printf("%s\n", buf); /* Local echo. */
  
  /*
  for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
    if (c == nc) continue; // Don't send to the sender.
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, buf, strlen(buf));
  }
  */

  mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, buf, strlen(buf));
}

static void broadcast_options_set(struct mg_connection *nc, const struct mg_str msg) {
//  struct mg_connection *c;
  char buf[500];
  char command_buf[100];
  char addr[32];
  bool is_continuous_requested = false;
  bool is_continuous_stop_requested = false;
  char* req;
  char* msec_timer_option;
  bool command_success = false;
  snprintf(command_buf, sizeof(command_buf), "%.*s", (int) msg.len, msg.p);
  mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
                      MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

  req = split_string(command_buf, " ", 0);
  msec_timer_option = split_string(command_buf, " ", 1);

  is_continuous_requested = strcmp(req, continuous_transmission_request)==0 ? true : false;
  is_continuous_stop_requested = strcmp(req, continuous_transmission_request_stop)==0 ? true : false;

  if(is_continuous_requested){
    mg_set_timer(nc, mg_time() + 0.001);
    command_success = true;
  } else if (is_continuous_stop_requested) {
    mg_set_timer(nc, mg_time() + 99999999);   // not the best solution :/
    command_success = true;
  }

  if(command_success){
    snprintf(buf, sizeof(buf), "%s: Command Accepted", addr);
    printf("%s\n", buf); /* Local echo. */
    mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, buf, strlen(buf));
  } else {
    snprintf(buf, sizeof(buf), "%s: Command Refused", addr);
    printf("%s\n", buf); /* Local echo. */
    mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, buf, strlen(buf));
  }

  
  /*
  for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
    if (c == nc) continue; // Don't send to the sender.
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, buf, strlen(buf));
  }
  */
}


static void broadcast(struct mg_connection *nc, const struct mg_str msg) {
  struct mg_connection *c;
  char buf[500];
  char addr[32];
  mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
                      MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

  snprintf(buf, sizeof(buf), "%s %.*s", addr, (int) msg.len, msg.p);
  printf("%s\n", buf); /* Local echo. */
  for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
    if (c == nc) continue; /* Don't send to the sender. */
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, buf, strlen(buf));
  }
}


static void broadcast_JSON_formatted_DATA(struct mg_connection *nc) {
  char buf[500];
  char addr[32];
  mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
                      MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

//  snprintf(buf, sizeof(buf), "%s %.*s", addr, (int) msg.len, msg.p);
  snprintf(buf, sizeof(buf), "{\"V1\":%1.3f,"
                             "\"V2\":%1.3f,"
                             "\"V3\":%1.3f,"
                             "\"V4\":%1.3f,"
                             "\"V5\":%1.3f,"
                             "\"V6\":%1.3f,"
                             "\"RA\":%1.3f,"
                             "\"LA\":%1.3f}",
                             channel_values[ADC_V1]/1000.0,
                             channel_values[ADC_V2]/1000.0,
                             channel_values[ADC_V3]/1000.0,
                             channel_values[ADC_V4]/1000.0,
                             channel_values[ADC_V5]/1000.0,
                             channel_values[ADC_V6]/1000.0,
                             channel_values[ADC_RA]/1000.0,
                             channel_values[ADC_LA]/1000.0);

  //printf("%s\n", buf); /* Local echo. */
  printf("%u\n", samples_taken++);
  mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, buf, strlen(buf));
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  switch (ev) {
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
      // New websocket connection. Tell only the current connected user
      broadcast_options(nc, mg_mk_str("supported commands: \"start_continuous [period(ms)]\", \"stop_continuous\""));
      break;
    }
    case MG_EV_WEBSOCKET_FRAME: {
      struct websocket_message *wm = (struct websocket_message *) ev_data;
      /* New websocket message. Tell everybody. */
      struct mg_str d = {(char *) wm->data, wm->size};
      broadcast_options_set(nc, d);
      break;
    }
    case MG_EV_TIMER: {
      // a new timer event, after the previous was set

      // set the next timer
      mg_set_timer(nc, mg_time() + 0.001);

      // send JSON formatted values on the ADC channels
      printf("timer triggered");
      broadcast_JSON_formatted_DATA(nc);

      break;
    }
    case MG_EV_HTTP_REQUEST: {
      mg_serve_http(nc, (struct http_message *) ev_data, s_http_server_opts);
      break;
    }
    case MG_EV_CLOSE: {
      /* Disconnect. Tell everybody. */
      if (is_websocket(nc)) {
        broadcast(nc, mg_mk_str("-- left"));
      }
      break;
    }
  }
}

int fd = -1;                                // used to open /dev/mem
void *h2f_lw_virtual_base;                  // the light weight buss base
volatile unsigned int * ADC_ptr = NULL;     // virtual address pointer to read ADC

int main(void) {
  struct mg_mgr mgr;
  struct mg_connection *nc;

  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  setvbuf(stdout, NULL, _IOLBF, 0);
  setvbuf(stderr, NULL, _IOLBF, 0);

  mg_mgr_init(&mgr, NULL);

  nc = mg_bind(&mgr, s_http_port, ev_handler);
  if (nc == NULL) {
    printf("Cannot start on port %s\n", s_http_port);
    return EXIT_FAILURE;
  }

  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = ".";  // Serve current directory
  s_http_server_opts.enable_directory_listing = "yes";

  printf("Started ECG server on port %s\n", s_http_port);

  /************ ADC mapping / setup ************/
  // Create virtual memory access to the FPGA light-weight bridge
  if ((fd = open_physical (fd)) == -1) {
    printf( "ERROR: could not open \"/dev/mem\"...\n" );
    return (-1);
  }
    
  if (!(h2f_lw_virtual_base = map_physical (fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN))) {
		printf( "ERROR: mmap1() failed...\n" );
		close( fd );
    return (-1);
  }
  
  // Set virtual address pointer to I/O port
  ADC_ptr = (unsigned int *) (h2f_lw_virtual_base + ADC_BASE);

  // Sets the ADC up to automatically perform conversions.
  *(ADC_ptr + 1) = 1;

  // C99 iterator
  int i;

/************ end ADC mapping / setup ************/

  while (s_signal_received == 0) {
    
    // check the refresh bit (bit 15) of port 1 to check for an update 
    if ((*ADC_ptr) & 0x8000){           
        for(i = 0; i < 8; i++){
            channel_values[i] = *(ADC_ptr+i) & ADC_VALUE_MASK;
        }
    }
    
    mg_mgr_poll(&mgr, 1);
  }

  // clean up
  mg_mgr_free(&mgr);
  unmap_physical (h2f_lw_virtual_base, LW_BRIDGE_SPAN);
  close_physical (fd);
  
  return 0;
}