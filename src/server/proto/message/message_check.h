/*
 * ngIRCd -- Message validation helpers
 */

#ifndef __message_check_h__
#define __message_check_h__

#include "portab.h"

struct _REQUEST;

typedef enum {
	MESSAGE_CHECK_OK = 0,
	MESSAGE_CHECK_NO_RECIPIENT,
	MESSAGE_CHECK_NO_TEXT,
	MESSAGE_CHECK_NEED_MORE_PARAMS,
	MESSAGE_CHECK_INVALID_CHANNEL
} MESSAGE_CHECK_RESULT;

GLOBAL MESSAGE_CHECK_RESULT Message_Check_Send PARAMS((const void *Client,
						      const struct _REQUEST *Req,
						      int ForceType,
						      bool SendErrors));
GLOBAL MESSAGE_CHECK_RESULT Message_Check_Join PARAMS((const void *Client,
						     const struct _REQUEST *Req));

#endif
