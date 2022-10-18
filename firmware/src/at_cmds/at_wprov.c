/**
 *
 * Copyright (c) 2019 Microchip Technology Inc. and its subsidiaries.
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <stddef.h>
#include <errno.h>

#include "at_cmd_app.h"

/*******************************************************************************
* Command interface prototypes
*******************************************************************************/
static ATCMD_STATUS _WPROVInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc);
static ATCMD_STATUS _WPROVCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _WPROVExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList);
static ATCMD_STATUS _WPROVUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc);


/*******************************************************************************
* Command parameters
*******************************************************************************/
static const ATCMD_HELP_PARAM paramID =
    {"ID", "Parameter ID number", ATCMD_PARAM_TYPE_CLASS_INTEGER, 0};

static const ATCMD_HELP_PARAM paramVAL =
    {"VAL", "Parameter value", ATCMD_PARAM_TYPE_CLASS_ANY, 0};

static const ATCMD_HELP_PARAM paramSTATE =
    {"STATE", "State of the provisioning feature", ATCMD_PARAM_TYPE_INTEGER,
        .numOpts = 2,
        {
            {"0", "Stop"},
            {"1", "Start"}
        }
    };

/*******************************************************************************
* Command examples
*******************************************************************************/

/*******************************************************************************
* Command descriptors
*******************************************************************************/
const AT_CMD_TYPE_DESC atCmdTypeDescWPROVC =
    {
        .pCmdName   = "+WPROVC",
        .cmdInit    = _WPROVInit,
        .cmdExecute = _WPROVCExecute,
        .cmdUpdate  = NULL,
        .pSummary   = "This command is used to configure the operation of the DCEs provisioning feature",
        .numVars    = 3,
        {
            {
                .numParams   = 0,
                .pParams     =
                {
                    NULL
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            },
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramID
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            },
            {
                .numParams   = 2,
                .pParams     =
                {
                    &paramID,
                    &paramVAL
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

const AT_CMD_TYPE_DESC atCmdTypeDescWPROV =
    {
        .pCmdName   = "+WPROV",
        .cmdInit    = NULL,
        .cmdExecute = _WPROVExecute,
        .cmdUpdate  = _WPROVUpdate,
        .pSummary   = "This command is used to control the operation of the DCEs provisioning feature",
        .numVars    = 2,
        {
            {
                .numParams   = 0,
                .pParams     =
                {
                    NULL
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            },
            {
                .numParams   = 1,
                .pParams     =
                {
                    &paramSTATE
                },
                .numExamples = 0,
                .pExamples   =
                {
                    NULL
                }
            }
        }
    };

/*******************************************************************************
* External references
*******************************************************************************/
extern ATCMD_APP_CONTEXT atCmdAppContext;

/*******************************************************************************
* Local defines and types
*******************************************************************************/
#define WPROVC_MAP_MAX_PARAMS     3

/*******************************************************************************
* Local data
*******************************************************************************/
static const ATCMD_STORE_MAP_ELEMENT wprovConfMap[] = {
    {1,  offsetof(ATCMD_APP_WPROV_CONF, mode),              ATCMD_STORE_TYPE_INT,        sizeof(int),       ATCMD_STORE_ACCESS_RW},
    {2,  offsetof(ATCMD_APP_WPROV_CONF, pin),               ATCMD_STORE_TYPE_STRING,     8,                 ATCMD_STORE_ACCESS_RW},
	{3,  offsetof(ATCMD_APP_WPROV_CONF, provPort),			ATCMD_STORE_TYPE_INT,	 	 sizeof(int), 		ATCMD_STORE_ACCESS_RW},
    {0,  0,                                                 ATCMD_STORE_TYPE_INVALID,    0,                 ATCMD_STORE_ACCESS_RW}
};

SYS_MODULE_OBJ netSrvcHdl = SYS_MODULE_OBJ_INVALID;

/****************
JSON
****************/

/** Max token size of JSON element. Token means that pair of key and value. */
#define JSON_MAX_TOKEN_SIZE 64
/** Max size of token name. */
#define JSON_MAX_NAME_SIZE 16

/**
 * \brief JSON type.
 */
enum json_type
{
	JSON_TYPE_NULL = 0,
	JSON_TYPE_OBJECT,
	JSON_TYPE_STRING,
	JSON_TYPE_BOOLEAN,
	JSON_TYPE_INTEGER,
	JSON_TYPE_REAL,
	JSON_TYPE_ARRAY,
	JSON_TYPE_MAX
};

/** \brief JSON data structure. */
struct json_obj
{
	/** Type of this data. */
	enum json_type type;
	/** Name of this data. */
	char name[JSON_MAX_NAME_SIZE];
	/** End pointer of JSON buffer. */
	char *end_ptr;

	/** Value of this JSON token. */
	union
	{
		/* String data. */
		char s[JSON_MAX_TOKEN_SIZE - JSON_MAX_NAME_SIZE];
		/* Boolean data. */
		int b;
		/* Fixed number data. */
		int i;
		/* Real number data. */
		double d;
		/* Object or Array data. */
		char *o; /* Start point of object. */
	} value;
};


enum json_cmd
{
	JSON_CMD_NONE = 0,
	JSON_CMD_ENTER_OBJECT,
	JSON_CMD_EXIT_OBJECT,
	JSON_CMD_ENTER_ARRAY,
	JSON_CMD_EXIT_ARRAY
};


/**
 *	@token is pair of name : value.
 *	@param buffer		 	: [in] Read out line
 *	@param dest				: [out] destination buffer
 *	@param cmd				: [out] 0 : none 1: enter object 2: exit object 3: enter array 4: exit array
 *	@return 		 			: start point of next token
 */
static char * _json_read_token(char *buffer, uint32_t buffer_size, char *dest, uint32_t dest_size, enum json_cmd *cmd)
{
	uint32_t dest_index = 0;
	char in_quote = 0;

	*cmd = JSON_CMD_NONE;

	dest[0] = '\0';
	
	for (; buffer_size > 0; buffer_size--) {
		char ch = *buffer++;
		if ( ch == '\"') {
			if(in_quote == 0) {
				in_quote = 1;
			} else if (in_quote == 1) {
				in_quote = 0;
			}
		}
		if (!in_quote) {
			if (ch == ',') {
				break;
			} else if (ch == '}') {
				*cmd = JSON_CMD_EXIT_OBJECT;
				break;
			} else if (ch == ']') {
				*cmd = JSON_CMD_EXIT_ARRAY;
				break;
			} else if(ch == '\t' || ch == ' ' || ch == '\r' || ch == '\n') {
				continue;
			} else if(ch == '\0') {
				return NULL;
			} else if( ch == '{') {
				*cmd = JSON_CMD_ENTER_OBJECT;
				break;
			} else if( ch == '[') {
				*cmd = JSON_CMD_ENTER_ARRAY;
				break;
			}
		}
		
		if (dest_index < dest_size) {
			dest[dest_index++] = ch;
		}
	}

	dest[dest_index] = '\0';

	return buffer;
}

static void _json_parse(char *data, char *ptr, enum json_cmd cmd, struct json_obj *out) 
{
	int i;
	int minus = 0;
	int exponent = -1;
	int num = 0;
	int find_point = 0;
	int real_size = 1;
	int minus_exp = 0;

	if(data[0] != '\0') {
		for (i = 0; *data != '\0' && *data != ':'; data++) {
			if (*data != '\"' && i < JSON_MAX_NAME_SIZE - 1) {
				out->name[i++] = *data;
			}
		}
		out->name[i] = '\0';
		out->type = JSON_TYPE_NULL;

		if (*data == ':') {
			data++;
			if (*data == '-') {
				minus = 1;
				data++;
			}
			if (*data == '\"') {
				out->type = JSON_TYPE_STRING;
				for (i = 0; *data != '\0'; data++) {
					if (*data != '\"' && i < JSON_MAX_TOKEN_SIZE - JSON_MAX_NAME_SIZE - 1) {
						out->value.s[i++] = *data;
					}
				}
				out->value.s[i] = '\0';
			} else if (*data >= '0' && *data <= '9') {
				for (; *data != '\0'; data++) {
					if (*data >= '0' && *data <= '9') {
						if (exponent < 0) {
							num = num * 10 + (*data & 0xF);
							if (find_point == 1) {
								real_size = 10;
							}
						} else {
							exponent = exponent * 10 + *data - '0';
						}
					} else if (*data == '.') {
						find_point = 1;
					} else if (*data == 'e' || *data == 'E') {
						exponent = 0;
					} else if (*data == '-' && exponent >= 0) {
						minus_exp = 1;
					}
				}
				
				if (find_point == 0 && exponent < 0) {
					out->type = JSON_TYPE_INTEGER;
					out->value.i = (minus) ? num * -1 : num;
				} else {
					out->type = JSON_TYPE_REAL;
					out->value.d = (minus) ? (double)num / (double)real_size * -1 : (double)num / (double)real_size;
					if (exponent >= 0) {
						if (minus_exp) {
							for (i = 0; i < exponent; i++) {
								out->value.d /= 10.0f;
							}
						} else {
							for (i = 0; i < exponent; i++) {
								out->value.d *= 10.0f;
							}
						}
					}
				}
			} else if (!strncmp(data, "true", 4)) {
				out->type = JSON_TYPE_BOOLEAN;
				out->value.b = 1;
				
			} else if (!strncmp(data, "false", 5)) {
				out->type = JSON_TYPE_BOOLEAN;
				out->value.b = 0;
				
			} else if(*data == '\0' || !strncmp(data, "null", 4)) {
				out->type = JSON_TYPE_NULL;
			} else {
				out->type = JSON_TYPE_STRING;
				for (i = 0; *data != '\0'; data++) {
					if (*data != '\"' && i < JSON_MAX_TOKEN_SIZE - JSON_MAX_NAME_SIZE - 1) {
						out->value.s[i++] = *data;
					}
				}
				out->value.s[i] = '\0';
			}
		}
	} else {
		out->name[0] = 0;
	}
	
	if (cmd == JSON_CMD_ENTER_OBJECT) {
		out->type = JSON_TYPE_OBJECT;
		for (; *ptr != '{'; ptr--);
		out->value.o = ptr;
	} else if (cmd == JSON_CMD_ENTER_ARRAY) {
		out->type = JSON_TYPE_ARRAY;
		for (; *ptr != '['; ptr--);
		out->value.o = ptr;
	}
}

int json_create(struct json_obj *obj, const char *data, int data_len)
{
	if (obj == NULL || data == NULL) {
		return -EINVAL;
	}
	for (; *data != '{'; data++, data_len--) {
		if (*data == '\0') {
			return -EINVAL;
		}
	}
	
	obj->type = JSON_TYPE_OBJECT;
	obj->name[0] = 0;
	obj->value.o = (char *)data;
	obj->end_ptr = (char *)data + data_len;
	
	return 0;
}

int json_get_child_count(struct json_obj *obj)
{
	char dest[2];
	enum json_cmd cmd;
	int child = 0;
	char *ptr;
	int depth = -1;
	
	if (obj == NULL || (obj->type != JSON_TYPE_OBJECT && obj->type != JSON_TYPE_ARRAY) || 
		obj->value.o == NULL) {
		return -EINVAL;	
	}
	
	for (ptr = obj->value.o; ; ) {
		ptr = _json_read_token(ptr, obj->end_ptr - ptr, dest, 1, &cmd);
		if (ptr == NULL) {
			break;
		}
		if (dest[0] == '\0' && cmd == JSON_CMD_NONE) {
			continue;
		}
		if (cmd == JSON_CMD_EXIT_ARRAY || cmd == JSON_CMD_EXIT_OBJECT) {
			depth--;
			if (depth < 0) {
				break;
			}
		} else {
			if (depth == 0) {
				/* Found members */
				child++;
			}
			if (cmd == JSON_CMD_ENTER_OBJECT || cmd == JSON_CMD_ENTER_ARRAY) {
				depth++;
			}
		}
	}
	
	return child;
}

int json_get_child(struct json_obj *obj, int index, struct json_obj *out)
{
	char dest[JSON_MAX_TOKEN_SIZE];
	enum json_cmd cmd;
	int child = 0;
	char *ptr;
	int depth = -1;
	
	if (obj == NULL || out == NULL || (obj->type != JSON_TYPE_OBJECT && obj->type != JSON_TYPE_ARRAY) ||
		obj->value.o == NULL) {
		return -EINVAL;
	}
	
	for (ptr = obj->value.o; ; ) {
		ptr = _json_read_token(ptr, obj->end_ptr - ptr, dest, JSON_MAX_TOKEN_SIZE - 1, &cmd);
		if (ptr == NULL) {
			break;
		}
		if (dest[0] == '\0' && cmd == JSON_CMD_NONE) {
			continue;
		}
		if (cmd == JSON_CMD_EXIT_ARRAY || cmd == JSON_CMD_EXIT_OBJECT) {
			depth--;
			if (depth < 0) {
				break;
			}
		} else {
			if (depth == 0) {
				/* Found members */
				if (child == index) {
					_json_parse(dest, ptr, cmd, out);
					out->end_ptr = obj->end_ptr;
					break;
				}
				child++;
			}
			if (cmd == JSON_CMD_ENTER_OBJECT || cmd == JSON_CMD_ENTER_ARRAY) {
				depth++;
			}
		}
	}
	
	return child;
}

int json_find(struct json_obj *obj, const char *name, struct json_obj *out)
{
	char dest[JSON_MAX_TOKEN_SIZE];
	enum json_cmd cmd;
	char *ptr;
	char *name_ptr = (char *)name;
	int depth = -1;
	
	if (obj == NULL || out == NULL || (obj->type != JSON_TYPE_OBJECT && obj->type != JSON_TYPE_ARRAY) ||
		obj->value.o == NULL || name == NULL || strlen(name) == 0) {
		return -EINVAL;
	}
	
	for (ptr = obj->value.o; ; ) {
		ptr = _json_read_token(ptr, obj->end_ptr - ptr, dest, JSON_MAX_TOKEN_SIZE - 1, &cmd);
		if (ptr == NULL) {
			break;
		}
		if (dest[0] == '\0' && cmd == JSON_CMD_NONE) {
			continue;
		}
		
		if (depth == 0) {
			_json_parse(dest, ptr, cmd, out);
			if (!strncmp(name_ptr, out->name, strlen(out->name)) &&
				(name_ptr[strlen(out->name)] == ':' || name_ptr[strlen(out->name)] == '\0')) {
				if (name_ptr[strlen(out->name)] == '\0') {
					return 0;
				} else {
					name_ptr += strlen(out->name) + 1;
					depth = 0;
					continue;
				}
			}
		}

		if (cmd == JSON_CMD_EXIT_ARRAY || cmd == JSON_CMD_EXIT_OBJECT) {
			if (depth == 0) {
				return -1;
			}
			depth--;
			if (depth < 0) {
				break;
			}
		} else if (cmd == JSON_CMD_ENTER_OBJECT || cmd == JSON_CMD_ENTER_ARRAY) {
			depth++;
		}
	}
	
	return -1;
}


/*******************************************************************************
* Local functions
*******************************************************************************/
static bool _ValidateChecksum(uint8_t *pPin)
{
    uint32_t accum = 0;

    accum += 3 * (*pPin++ - '0');
    accum += 1 * (*pPin++ - '0');
    accum += 3 * (*pPin++ - '0');
    accum += 1 * (*pPin++ - '0');
    accum += 3 * (*pPin++ - '0');
    accum += 1 * (*pPin++ - '0');
    accum += 3 * (*pPin++ - '0');
    accum += 1 * (*pPin++ - '0');

    return (0 == (accum % 10));
}

uint8_t response_buffer[1024] = {0};
int32_t response_buffer_length = 0;

static void _WPROV_DataUpdate(uint8_t buffer[]) 
{
    struct json_obj root, child, sub;
	bool error = false;

    if (buffer) 
    {
        /* Creating JSON object to parse incoming JSON data */
        if (!json_create(&root, (const char*) buffer, strlen((const char*) buffer))) 
        {
        	ATCMD_Printf("JSON created\n");
			
			if (!json_find(&child, "SSID", &sub)) 
			{
				if (strlen(sub.value.s) <= sizeof (atCmdAppContext.wstaConf.ssid)) 
				{
					memcpy(atCmdAppContext.wstaConf.ssid, sub.value.s, strlen(sub.value.s));
				} 
				else
				{
					error = true;
				}
			} 
			else
			{
				error = true;
			}
        }
		else if (!strncmp((const char *) buffer, "apply", 5)) 
		{		 
			char * p = strtok((char *) buffer, ",");
			p = strtok(NULL, ",");
			if (p)
			{
				atCmdAppContext.wstaConf.ssid[0] = strlen(p);
				ATCMD_Printf("SSID Len %d\n", strlen(p));
				strcpy((char *)&atCmdAppContext.wstaConf.ssid[1], p);
			}
		
			p = strtok(NULL, ",");
			if (p) 
			{
				char appAuthType = *p - '0';
				if (appAuthType == 1) /* 1-Open */
				{ 
					atCmdAppContext.wstaConf.secType = WDRV_PIC32MZW_AUTH_TYPE_OPEN;
				} 
				else if (appAuthType == 2) /* 2-WPA2 */
				{
					atCmdAppContext.wstaConf.secType = WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL;
					p = strtok(NULL, ",");
					if (p) 
					{
						atCmdAppContext.wstaConf.credentials[0] = strlen(p);
						ATCMD_Printf("Credentials len %d\n", strlen(p)); 
						strcpy((char *)&atCmdAppContext.wstaConf.credentials[1], p);
					}
				} 
				else
				{
					error = true;
				}
			}
		
			/* Verifying received data error */
			if (!error) 
			{
				ATCMD_Printf("Success\n");
			}
			else 
			{
				ATCMD_Printf(" Wrong Command\n");
			}
		}
		else if (!strncmp((const char *) buffer, "AT+", 3)) 
		{
			if (!strncmp((const char *) buffer, "AT+WSCN", 7))
			{
				memset(response_buffer, 0, sizeof(response_buffer));
				response_buffer_length = 0;
				atCmdAppContext.respond_to_app = 1;
			}
			if (!strncmp((const char *) buffer, "AT+WSTAC", 8))
			{
				memset(response_buffer, 0, sizeof(response_buffer));
				response_buffer_length = 0;
				atCmdAppContext.respond_to_app = 1;
			}
			ATCMD_ParseCommandLine((char *)buffer);
		}
    }
}

void _APTcpClientCallback(uint32_t event, void *data, void* cookie)
{
	uint8_t buffer[1500] = {0};
	int32_t length = 1500;
	
    switch (event)
    {
 	   case SYS_NET_EVNT_CONNECTED:
    	{
//			ATCMD_Print("UP\n", strlen("UP\n"));
			break;
    	}
 	   case SYS_NET_EVNT_RCVD_DATA:
    	{
//			ATCMD_Print("Data\n", strlen("Data\n"));
			length = SYS_NET_RecvMsg(netSrvcHdl, buffer, length);
//			ATCMD_Printf("Msg %s Length %d\n", buffer, length);
			_WPROV_DataUpdate(buffer);
			break;
    	}
    }
}



/*******************************************************************************
* Command init functions
*******************************************************************************/
static ATCMD_STATUS _WPROVInit(const AT_CMD_TYPE_DESC* pCmdTypeDesc)
{
    memset(&atCmdAppContext.wprovConf, 0, sizeof(ATCMD_APP_WPROV_CONF));
    memset(&atCmdAppContext.wprovState, 0, sizeof(ATCMD_APP_WPROV_STATE));

	/*
	 * Set default Provisioning Mode to SoftAP mode.
	 */
	 
	atCmdAppContext.wprovConf.mode = 2;
	atCmdAppContext.wprovConf.provPort = 7777;

    return ATCMD_STATUS_OK;
}

/*******************************************************************************
* Command execute functions
*******************************************************************************/
static ATCMD_STATUS _WPROVCExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    if (0 == numParams)
    {
        int id;

        /* Dump all configuration elements */

        for (id=1; id<=WPROVC_MAP_MAX_PARAMS; id++)
        {
            /* Read the element from the configuration structure */

            ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, wprovConfMap, &atCmdAppContext.wprovConf, id);
        }

        return ATCMD_STATUS_OK;
    }
    else if (1 == numParams)
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        /* Access the element in the configuration structure */

        if (false == ATCMD_StructStorePrint(pCmdTypeDesc->pCmdName, wprovConfMap, &atCmdAppContext.wprovConf, pParamList[0].value.i))
        {
            return ATCMD_STATUS_STORE_ACCESS_FAILED;
        }
    }
    else if (2 == numParams)
    {
        /* Check the parameter types are correct */

        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 2, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        /* If started then block write access to all elements */

        if (0 != atCmdAppContext.wapConnState.wapState)
        {
            return ATCMD_STATUS_STORE_UPDATE_BLOCKED;
        }

        if (2 == pParamList[0].value.i)
        {
            if (8 == pParamList[1].length)
            {
                if (false == _ValidateChecksum(pParamList[1].value.p))
                {
                    return ATCMD_STATUS_INVALID_PARAMETER;
                }
            }
            else if (4 != pParamList[1].length)
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }
        }

        /* Access the element in the configuration structure */

        if (0 == ATCMD_StructStoreWriteParam(wprovConfMap, &atCmdAppContext.wprovConf, pParamList[0].value.i, &pParamList[1]))
        {
            return ATCMD_STATUS_STORE_ACCESS_FAILED;
        }
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    return ATCMD_STATUS_OK;
}

static ATCMD_STATUS _WPROVUpdate(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const AT_CMD_TYPE_DESC* pCurrentCmdTypeDesc)
{
    if (ATCMD_APP_STATE_PROV_AP_STARTED != atCmdAppContext.appState)
    {
    	return ATCMD_STATUS_OK;
    }

	SYS_NET_Task(netSrvcHdl);

	return ATCMD_STATUS_PENDING;
}

static ATCMD_STATUS _WPROVExecute(const AT_CMD_TYPE_DESC* pCmdTypeDesc, const int numParams, ATCMD_PARAM *pParamList)
{
    ATCMD_STATUS retStatus;

    if (0 == numParams)
    {
        if ((ATCMD_APP_STATE_WPS_STARTED == ATCMD_APPStateMachineCurrentState()) || (ATCMD_APP_STATE_PROV_AP_STARTED == ATCMD_APPStateMachineCurrentState()))
        {
            ATCMD_Print("+WPROV:1\r\n", 10);
        }
        else
        {
            ATCMD_Print("+WPROV:0\r\n", 10);
        }

        return ATCMD_STATUS_OK;
    }
    else if (1 == numParams)
    {
        if (false == ATCMD_ParamValidateTypes(pCmdTypeDesc, 1, numParams, pParamList))
        {
            return ATCMD_STATUS_INVALID_PARAMETER;
        }

        if ((0 == atCmdAppContext.wprovConf.mode) || (1 == atCmdAppContext.wprovConf.mode))
        {
            /* WPS */

            if (0 == pParamList[0].value.i)
            {
                /* WPS Stop */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STOP_WPS, false))
                {
                    return ATCMD_APP_STATUS_PROV_WPS_STOP_REFUSED;
                }

                /* TODO */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_STOP_WPS, true))
                {
                    return ATCMD_APP_STATUS_PROV_WPS_STOP_FAILED;
                }
            }
            else if (1 == pParamList[0].value.i)
            {
                /* WPS Start */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_START_WPS, false))
                {
                    return ATCMD_APP_STATUS_PROV_WPS_START_REFUSED;
                }

                /* TODO */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_START_WPS, true))
                {
                    return ATCMD_APP_STATUS_PROV_WPS_START_FAILED;
                }
            }
            else
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }
        }
        else if (2 == atCmdAppContext.wprovConf.mode)
        {
            /* Web page provisioning */

            if (0 == pParamList[0].value.i)
            {
                /* Stop */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_PROV_AP_STOPPING, false))
                {
                    return ATCMD_APP_STATUS_PROV_AP_STOP_REFUSED;
                }

                retStatus = ATCMD_WAP_Stop();

                if (ATCMD_STATUS_OK != retStatus)
                {
                    return retStatus;
                }

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_PROV_AP_STOPPING, true))
                {
                    return ATCMD_APP_STATUS_PROV_AP_STOP_FAILED;
                }
				ATCMD_Print("+WPROVEXIT\r\n", 12);
            }
            else if (1 == pParamList[0].value.i)
            {
                /* Start */

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_PROV_AP_STARTING, false))
                {
                    return ATCMD_APP_STATUS_PROV_AP_START_REFUSED;
                }

                retStatus = ATCMD_WAP_Start(true);

                if (ATCMD_STATUS_OK != retStatus)
                {
                    return retStatus;
                }

                if (false == ATCMD_APPStateMachineEvent(ATCMD_APP_EVENT_PROV_AP_STARTING, true))
                {
                    return ATCMD_APP_STATUS_PROV_AP_START_FAILED;
                }
#if 1
				if (netSrvcHdl == SYS_MODULE_OBJ_INVALID)
				{
					SYS_NET_Config sSysNetCfg = {0};
			
					/* Open a TCP Socket via the NET Service */
					memset(&sSysNetCfg, 0, sizeof (sSysNetCfg));
				
					sSysNetCfg.mode = SYS_NET_MODE_SERVER;
				
					sSysNetCfg.ip_prot = SYS_NET_IP_PROT_TCP;
				
					sSysNetCfg.enable_reconnect = 1;
				
					sSysNetCfg.enable_tls = 0;
				
					sSysNetCfg.port = atCmdAppContext.wprovConf.provPort;

					sSysNetCfg.intf = SYS_NET_INTF_WIFI;
				
					netSrvcHdl = SYS_NET_Open(&sSysNetCfg, _APTcpClientCallback, NULL);

					if (netSrvcHdl != SYS_MODULE_OBJ_INVALID)
					{
						return ATCMD_STATUS_OK;							
					}
					else
					{
						return ATCMD_STATUS_ERROR;
					}
				}
#endif
            }
            else
            {
                return ATCMD_STATUS_INVALID_PARAMETER;
            }
        }

        atCmdAppContext.wprovState.wprovState = pParamList[0].value.i;
    }
    else
    {
        return ATCMD_STATUS_INCORRECT_NUM_PARAMS;
    }

    return ATCMD_STATUS_PENDING;
}

/*******************************************************************************
* Command update functions
*******************************************************************************/
