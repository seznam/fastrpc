/*
 * FastRPC -- Fast RPC library compatible with XML-RPC * Copyright (C) 2005-7  Seznam.cz, a.s.
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
 *
 * FILE             $Id: frpchttpio.cc,v 1.13 2011-02-11 08:56:17 burlog Exp $
 *
 * DESCRIPTION      HTTP I/O
 *
 * AUTHOR           Vasek Blazek <blazek@firma.seznam.cz>
 *                  Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *          2005-02-07
 *                  First draft.
 *          2005-02-22
 *                  Transformed for FastRPC
 */

#include "nonglibc.h"

#include <sstream>
#include <algorithm>

#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <stdio.h>

#include "frpcsocket.h"
#include "frpchttpio.h"
#include <frpchttperror.h>
#include <frpcprotocolerror.h>
#include <frpcresponseerror.h>
#include <frpchttpclient.h>

// check for MSG_NOSIGNAL
#ifndef MSG_NOSIGNAL
#    define MSG_NOSIGNAL 0
#endif

using namespace FRPC;

HTTPIO_t::~HTTPIO_t() {
    // check whether socket is valid fd and close it
    if (fd > -1) TEMP_FAILURE_RETRY(close(fd));
}

std::vector<std::string>
HTTPIO_t::splitBySpace(const std::string &line, size_t upto)
{
    std::vector<std::string> parts;

    --upto;
    // indikuje, jestli je pøedchozí znak nebílý (1) nebo bílý (0)
    bool state = false;
    std::string::size_type begin = 0;
    std::string::size_type length = line.length();
    // pro v¹echny znaky øetìzce
    for (std::string::size_type i = 0; i < length; ++i)
    {
        if (!isspace(line[i]))
        {
            // pokud jde o nebílý znak
            // pokud pøedchozí byl bílý, nastavíme zaèátek na aktuální
            if (!state)
                begin = i;
            state = true;
        }
        else
        {
            // bílý znak
            // pokud byl pøedchozí znak nebílý, vlo¾íme "slovo" do výstupu
            if (state)
                parts.push_back(line.substr(begin, i - begin));
            // pokud jsme ji¾ vybrali slova do maxima - 1, tak
            // prípadný zbytek vlo¾íme jako poslední slovo
            if (parts.size() == upto)
            {
                if ((length - ++i) > 0)
                    parts.push_back(line.substr(i, length - i));
                return parts;
            }
            state = false;
        }
    }
    // jsme na konci øetìzce
    // pokud øetìzec konèí nebílým znakem, vlo¾íme "slovo" do výstupu
    if (state)
        parts.push_back(line.substr(begin, length - begin));

    return parts;
}

int HTTPIO_t::getHeaderValue(const std::string &line, std::string &name,
                             std::string &value)
{
    // check for empty line
    if (line.empty())
        return -1;
    // check for contignuous line
    if (isspace(line[0]))
    {
        // OK, previous line continues here
        std::string::const_iterator i = line.begin();
        while ((i != line.end()) && isspace(*i))
            ++i;
        value.push_back(' ');
        value.append(i, line.end());
        return 0;
    }

    // najdeme separátor jména od hodnoty
    std::string::size_type idx = line.find(':');
    // ¹patnì formulovaný øádek -> chyba
    if (idx == std::string::npos)
        return -1;
    // vyøízneme jméno polo¾ky
    name = line.substr(0, idx);

    // ve zbytku najdeme první nebílý znak
    std::string::size_type length = line.length();
    for (idx += 1; idx < length; ++idx)
        if (!isspace(line[idx]))
            break;

    // pokud dojedeme na konec øetìzce -> konec
    if (idx == length)
        return 0;
    // najdeme první nebílý znak z konce
    while (isspace(line[length - 1]))
        --length;

    // vyøízneme hodnotu
    value = line.substr(idx, length - idx);
    return 0;
}

namespace
{
const unsigned int HTTP_BUFF_LENGTH = 1 << 16;
}

std::string HTTPIO_t::readLine(bool checkLimit)
{
    // buffer pro ètení ze socketu.
    char buff[HTTP_BUFF_LENGTH];

    // celkový buffer
    std::string lineBuff;

    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    for (;;)
    {
        // èekání na data na socketu
        int ready = TEMP_FAILURE_RETRY(
                poll(&pfd, 1, readTimeout < 0 ? -1 : readTimeout));

        switch (ready)
        {
        case 0:
            throw ProtocolError_t(HTTP_TIMEOUT, "Timeout while reading.");

        case -1:
            // other error
            STRERROR_PRE();
            throw ProtocolError_t(HTTP_SYSCALL, "Syscall error: <%d, %s>.",
                              ERRNO, STRERROR(ERRNO));
        }

        // pøeèteme data ze socketu, ale neostraníme je z receive bufferu
        int bytes = TEMP_FAILURE_RETRY(
                recv(fd, buff, HTTP_BUFF_LENGTH, MSG_PEEK | MSG_NOSIGNAL));
        switch (bytes)
        {
        case 0:
            // protìjsí strana zavøela spojení
            throw ProtocolError_t(HTTP_CLOSED,
                                  "Connection closed by foreign host");

        case -1:
            // other error
            STRERROR_PRE();
            throw ProtocolError_t(HTTP_SYSCALL, "Syscall error: <%d, %s>.",
                              ERRNO, STRERROR(ERRNO));
        }

        // hledáme <LF> v pøeèteném bloku dat
        char *end = static_cast<char*>(memchr(buff, '\n', bytes));
        if (end)
        {
            // pokud je znak <LF> nalezen, vyèti pouze potøebný poèet znakù
            unsigned int toRead = end - buff + 1;
            // check line size limit
            if (checkLimit && (lineSizeLimit >= 0) &&
                    ((lineBuff.length() + toRead)
                     > static_cast<unsigned int>(lineSizeLimit)))
            {
                throw ProtocolError_t
                (HTTP_LINE_TOO_LONG,"Security limit exceeded: line "
                 "is too long ('%u' > '%d')",
                 lineBuff.length() + toRead,
                 lineSizeLimit);
            }

            bytes = TEMP_FAILURE_RETRY(recv(fd, buff, toRead, MSG_NOSIGNAL));
            switch (bytes)
            {
            case 0:
                // protìjsí strana zavøela spojení
                throw ProtocolError_t(HTTP_CLOSED,
                                  "Connection closed by foreign host");

            case -1:
                // other error
                STRERROR_PRE();
                throw ProtocolError_t(HTTP_SYSCALL, "Syscall error: <%d, %s>.",
                                  ERRNO, STRERROR(ERRNO));

            default:
                // vyèteme v¹echny znaky vèetnì <LF>, øe»ezec prøilepíme
                // na konec øádky a uma¾eme z prava vøechny <CR>
                lineBuff.append(buff, bytes - 1);
                size_t len;
                while ((len = lineBuff.length()))
                {
                    if (*lineBuff.rbegin() == '\r')
                        lineBuff.resize(len - 1);
                    else
                        break;
                }
                // OK, terminate reading
                return lineBuff;
            }
        }
        else
        {
            // jinak vyèteme v¹echna dostupná data
            unsigned int toRead = bytes;
            // check line size limit
            if (checkLimit && (lineSizeLimit >= 0) &&
                    ((lineBuff.length() + toRead)
                     > static_cast<unsigned int>(lineSizeLimit)))
            {
                throw ProtocolError_t
                (HTTP_LINE_TOO_LONG, "Security limit exceeded: line "
                 "is too long ('%u' > '%d')",
                 lineBuff.length() + toRead,
                 lineSizeLimit);
            }

            bytes = TEMP_FAILURE_RETRY(recv(fd, buff, toRead, MSG_NOSIGNAL));
            switch (bytes)
            {
            case 0:
                // protìjsí strana zavøela spojení
                throw ProtocolError_t(HTTP_CLOSED,
                                  "Connection closed by foreign host");

            case -1:
                // other error
                STRERROR_PRE();
                throw ProtocolError_t(HTTP_SYSCALL, "Syscall error: <%d, %s>.",
                                  ERRNO, STRERROR(ERRNO));

            default:
                // pøilepíme øetìzec na konec øádky a jdeme na dal¹í ètení
                lineBuff.append(buff, bytes);
                break;
            }
        }
    }
}

void HTTPIO_t::sendData(const char *data, size_t length, bool watchForResponse)
{
    // zjistíme, kolik máme poslat
    if (!length)
        return;

    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLOUT;

    if (watchForResponse)
        pfd.events |= POLLIN;

    for (;;)
    {
        int ready = TEMP_FAILURE_RETRY(
                poll(&pfd, 1, writeTimeout < 0 ? -1 : writeTimeout));

        switch (ready)
        {
        case 0:
            throw ProtocolError_t(HTTP_TIMEOUT, "Timeout while writing.");

        case -1:
            // other error
            STRERROR_PRE();
            throw ProtocolError_t(HTTP_SYSCALL, "Syscall error: <%d, %s>.",
                              ERRNO, STRERROR(ERRNO));
        }

        // watch for read data if asked to do so
        if (watchForResponse && (pfd.revents & POLLIN))
            throw ResponseError_t();

        int toWrite = (length > HTTP_BUFF_LENGTH)
                      ? HTTP_BUFF_LENGTH : length;
        int bytes = TEMP_FAILURE_RETRY(send(fd, data, toWrite, MSG_NOSIGNAL));
        switch (bytes)
        {
        case 0:
            // ach jo, nic jsme nezapsali, tak to zkusíme znova
            continue;
        case -1:

            if (watchForResponse && (ERRNO == EPIPE))
            {
                throw ResponseError_t();
            }
            // other error
            STRERROR_PRE();
            throw ProtocolError_t(HTTP_SYSCALL, "Syscall error: <%d, %s>.",
                              ERRNO, STRERROR(ERRNO));

        default:
            // pøipravíme zapisování dal¹ích dat
            length -= bytes;
            data += bytes;
            // pokud není co zapsat -> konec
            if (!length)
                return;
        }
    }

    return;
}

void HTTPIO_t::readBlock(long int contentLength_, DataSink_t &data)
{
    // do not read empty content
    // -1 means read until end-of-connection
    if (contentLength_ == 0)
        return;
    unsigned long int contentLength = contentLength_;

    // check if content fits the max block size
    if ((bodySizeLimit >= 0) && (contentLength_ > bodySizeLimit))
        throw ProtocolError_t(HTTP_BODY_TOO_LONG, "Security limit exceeded: line "
         "is too long ('%ld' > '%d')",
         contentLength_, bodySizeLimit);

    // buffer pro cteni ze socketu.
    // if contentLength_ == -1 (reading till the end of connection) -> max buff size is HTTP_BUFF_LENGTH
    size_t buffer_size = contentLength_ > 0 ? contentLength_ : HTTP_BUFF_LENGTH;

    char buff[HTTP_BUFF_LENGTH];
    std::string string_buffer;
    string_buffer.reserve(buffer_size);

    size_t read_bytes = 0;

    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    for (;;)
    {
        // èekání na data na socketu
        int ready = TEMP_FAILURE_RETRY(
                poll(&pfd, 1, (readTimeout < 0) ? -1 : readTimeout));

        switch (ready)
        {
        case 0:
            throw ProtocolError_t(HTTP_TIMEOUT, "Timeout while reading.");

        case -1:
            // other error
            STRERROR_PRE();
            throw ProtocolError_t(HTTP_SYSCALL, "Syscall error: <%d, %s>.",
                              ERRNO, STRERROR(ERRNO));
        }

        // pøeèteme data ze socketu
        int toRead = (contentLength_ < 0 || contentLength > HTTP_BUFF_LENGTH)
                     ? HTTP_BUFF_LENGTH : contentLength;

        int bytes = TEMP_FAILURE_RETRY(recv(fd, buff, toRead, MSG_NOSIGNAL));
        read_bytes += bytes;
        switch (bytes)
        {
        case 0:
            // protìjsí strana zavøela spojení
            if (contentLength_ < 0) {
                data.write(string_buffer.c_str(), read_bytes);
                return; //done
            }
            throw ProtocolError_t(HTTP_CLOSED,
                                  "Connection closed by foreign host");

        case -1:
            // other error
            STRERROR_PRE();
            throw ProtocolError_t(HTTP_SYSCALL, "Syscall error: <%d, %s>.",
                              ERRNO, STRERROR(ERRNO));

        default:
            // pøilepíme data na konec dosud pøeètených dat
            if (contentLength_ >= 0)
                contentLength -= bytes;
            // test for maxblocksize
            if (bodySizeLimit >= 0 && read_bytes
                    > static_cast<unsigned long int>(bodySizeLimit))
                throw ProtocolError_t
                (HTTP_BODY_TOO_LONG, "Security limit exceeded: content "
                 "is too large (%u > %d)",
                 read_bytes, bodySizeLimit);

            if (read_bytes > string_buffer.capacity())
                string_buffer.reserve(read_bytes);

            string_buffer.append(buff, bytes);
            // pokud ji¾ není co zapsat -> konec
            if (!contentLength) {
                data.write(string_buffer.c_str(), read_bytes);
                return;
            }
        }
    }
}

void HTTPIO_t::readHeader(HTTPHeader_t &header)
{
    for (;;)
    {
        std::string line (readLine());
        // empty header terminates header
        if (line.empty())
            break;
        std::string name;
        std::string value;
        // parse header line
        if (getHeaderValue(line, name, value))
            // oops, nìkdo nám poslal binec...
            throw ProtocolError_t(HTTP_VALUE, "Invalid header line '%s'/",
                              line.substr(0, 30).c_str());
        if (name.empty())
        {
            // continuing line
            if (header.empty())
                // but no line to attach
                throw ProtocolError_t(HTTP_VALUE, "Invalid header line '%s'/",
                                  line.substr(0, 30).c_str());

            header.appendValue(value);
        }
        else
        {
            header.add(name, value);
        }
    }
}

long int HTTPIO_t::readChunkSize()
{
    std::string line(readLine());
    std::istringstream is(line);
    long int chunkSize;
    if (!(is >> std::hex >> chunkSize) || (chunkSize < 0))
        // reading of chunk size failed
        throw ProtocolError_t(HTTP_VALUE, "Bad chunk size: '%s'.",
                          line.substr(0, 30).c_str());
    return chunkSize;
}

void HTTPIO_t::readChunkedContent(HTTPHeader_t &header, DataSink_t &data)
{
    // while we have non-zero chunk size
    while (long int chunkSize = readChunkSize())
    {
        // check for body limit
        if ((bodySizeLimit >= 0) &&
                ((data.written() + chunkSize)
                 > static_cast<unsigned int>(bodySizeLimit)))
            throw ProtocolError_t
            (HTTP_BODY_TOO_LONG, "Security limit exceeded: line "
             "is too long ('%ld > %d)",
             data.written() + chunkSize,
             bodySizeLimit);

        // read chunk
        readBlock(chunkSize, data);

        // read CRLF after chunk
        std::string emptyLine(readLine());
        if (!emptyLine.empty())
            throw ProtocolError_t(HTTP_VALUE,
                              "Chunk terminator should be empty, "
                              "not '%s'.", emptyLine.substr(0, 30).c_str());
    }

    // read trailer
    readHeader(header);
}

void HTTPIO_t::readContent(HTTPHeader_t &header, DataSink_t &data,
                           bool request)
{
    std::string value;
    if (!header.get(HTTP_HEADER_CONTENT_LENGTH, value))
    {
        // got content-length => read given number of octets
        long int contentLength;
        std::istringstream is(value);
        if (!(is >> contentLength) || (contentLength < 0))
            throw ProtocolError_t(HTTP_VALUE,
                              "Invalid content length header '%s'.",
                              value.substr(0, 30).c_str());

        // check for body limit
        if ((bodySizeLimit >= 0) && (contentLength > bodySizeLimit))
            throw ProtocolError_t
            (HTTP_BODY_TOO_LONG, "Security limit exceeded: line "
             "is too long ('%ld > %d)",
             contentLength, bodySizeLimit);

        // tell sink how much data it will be fed
        //data.await(contentLength);

        // read body content
        readBlock(contentLength, data);
    }
    else if (!header.get(HTTP_HEADER_TRANSFER_ENCODING, value))
    {
        // got transfer-encoding
        std::transform(value.begin(), value.end(), value.begin(),
                       toupper);
        if (value == "CHUNKED")
        {
            // 'chunked' Transfer-Encoding => read all chunks
            readChunkedContent(header, data);
        }
        else
        {
            throw ProtocolError_t(HTTP_VALUE,
                              "Invalid content-transfer-encoding "
                              "header '%s'.", value.substr(0, 30).c_str());
        }
    }
    else if (!request)
    {
        // we cannot read request until end of conn, because it
        // would never end. we can read only responses to the end.
        // read body content until end of connection
        return readBlock(-1, data);
    }
}
