#ifndef STREAMLNK_H
#define STREAMLNK_H


typedef enum {
	SLNK_OK = 0,
	SLNK_RESET,
	SLNK_PARSE_ERROR,
	SLNK_OOM
} streamlnkresult;


int btm_rx_stream(long lPort);

#endif
