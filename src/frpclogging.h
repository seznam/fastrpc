/*
 * FastRPC -- Fast RPC library compatible with XML-RPC
 * Copyright (C) 2005-7  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Radlicka 2, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:fastrpc@firma.seznam.cz
 */

#include <string>
#include <frpcplatform.h>

#ifndef FRPCLOGGING_H
#define FRPCLOGGING_H

namespace FRPC {
// forwards
struct URL_t;
class Value_t;
class Array_t;
class HTTPHeader_t;

enum class LogEvent_t {
    CALL_START,
    CALL_SUCCESS,
    CALL_FAULT,
    CALL_ERROR,
};

union LogEventData_t {
    LogEventData_t() = default;

    struct CallBasics_t {
        const char *methodName;
        const Array_t *params;
        const URL_t *url;
    };

    struct CallStart_t: CallBasics_t {
    };

    struct CallSuccess_t: CallBasics_t {
        const Value_t *response;
        const HTTPHeader_t *responseHeaders;
    };

    struct CallFault_t: CallBasics_t {
        int statusCode;
        const std::string *msg;
        const HTTPHeader_t *responseHeaders;
    };

    struct CallError_t: CallBasics_t {
        const char *what;
        const HTTPHeader_t *responseHeaders;
    };

    CallStart_t callStart;
    CallSuccess_t callSuccess;
    CallFault_t callFault;
    CallError_t callError;
};

using LoggerFn_t = void (*)(LogEvent_t event, LogEventData_t &eventData, void *loggerData);

/**
 * Set logger callback function. The callback will be called for every loggable event.
 * This function is not reentrant nor thread-safe so it is best to call it right after
 * the initFastrpc() function.
 */
void FRPC_DLLEXPORT setLoggerCallback(LoggerFn_t loggerFn, void *loggerData);

} // namespace FRPC

#endif
