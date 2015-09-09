#ifndef BSHTM_H_
#define BSHTM_H_

#include <common.h>
#include "cdata.h"

#define CAPTCHA_URL_PREFIX	"CaptchaImage.aspx?guid="
#define __VIEWSTATE_PREFIX	"id=\"__VIEWSTATE\" value="
#define __VIEWSTATE_DATA_LEN	412

#define __EVENTVALIDATION_PREFIX	"id=\"__EVENTVALIDATION\" value="
#define __EVENTVALIDATION_LEN		156

#define SEESION_ID_PRIFIX	"Set-Cookie: ASP.NET_SessionId="
#define SEESION_ID_LEN 24

extern void set_bshtm_ops(cdata *d);
#endif

