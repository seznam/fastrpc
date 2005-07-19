/*
* FILE             $Id: frpchttp.cc,v 1.1 2005-07-19 13:02:53 vasek Exp $
*
* DESCRIPTION      HTTP Base types
*
* AUTHOR           Vasek Blazek <blazek@firma.seznam.cz>
*
* Copyright (C) Seznam.cz a.s. 2005
* All Rights Reserved
*
* HISTORY
*          2005-02-07
*                  First draft.
*/

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <algorithm>
#include <sstream>
#include <strstream>

#include <frpchttp.h>
#include <frpchttperror.h>
#include <frpctypeerror.h>

using namespace FRPC;

namespace
{
std::string polishName(const std::string &name)
{
    std::string ret;
    ret.reserve(name.length());
    for (std::string::const_iterator it = name.begin(); it < name.end(); ++it)
    {
        if (isalnum(*it))
        {
            if ((it != name.begin()) && (isalnum(*(it - 1))))
                ret.push_back(tolower(*it));
            else
                ret.push_back(toupper(*it));
        }
        else
            ret.push_back(*it);
    }

    return ret;
}
}

int HTTPHeader_t::get
    (const std::string &name_, std::string &value,
            unsigned int index) const
    {
        std::string name(polishName(name_));
        unsigned int current = 0;
        for (HeaderMap_t::const_iterator iheader = header.begin();
                iheader != header.end(); ++iheader)
        {
            if (name == iheader->first)
            {
                if (index == current++)
                {
                    value = iheader->second;
                    return 0;
                }
            }
        }
        return -1;
    }

std::vector<std::string>
HTTPHeader_t::getList(const std::string &name_) const
{
    std::vector<std::string> out;
    std::string name(polishName(name_));
    for (HeaderMap_t::const_iterator iheader = header.begin();
            iheader != header.end(); ++iheader)
        if (name == iheader->first)
            out.push_back(iheader->second);

    // OK
    return out;
}

void HTTPHeader_t::set
    (const std::string &name_, const std::string &value,
            bool create, unsigned int index)
{
    std::string name(polishName(name_));
    unsigned int current = 0;
    for (HeaderMap_t::iterator iheader = header.begin();
            iheader != header.end(); ++iheader)
    {
        if (name == iheader->first)
        {
            if (index == current++)
            {
                iheader->second = value;
                return;
            }
        }
    }
    if (create)
        header.push_back(Header_t(name, value));

}

void HTTPHeader_t::add
    (const std::string &name, const std::string &value)
{
    header.push_back(Header_t(polishName(name), value));
}

int HTTPHeader_t::remove
    (const std::string &name_, unsigned int index)
{
    std::string name(polishName(name_));
    unsigned int current = 0;
    for (HeaderMap_t::iterator iheader = header.begin();
            iheader != header.end(); ++iheader)
    {
        if (name == iheader->first)
        {
            if (index == current++)
            {
                header.erase(iheader);
                return 0;
            }
        }
    }
    return -1;
}

void HTTPHeader_t::appendValue(const std::string &value)
{
    if (header.empty())
        throw HTTPError_t(HTTP_VALUE, "No header field to append to.");
    header.back().second.append(value);
}

std::ostream& FRPC::operator<<(std::ostream &os,
                                      const HTTPHeader_t &header)
{
    // serialize header
    for (HTTPHeader_t::HeaderMap_t::const_iterator
            iheader = header.header.begin();
            iheader != header.header.end(); ++iheader)
        os << iheader->first << ": " << iheader->second << "\r\n";

    // pass stream
    return os;
}

namespace
{
    const std::string HTTP_SCHEMA("http://");

    inline bool nocaseCmp(char l, char r)
    {
        return toupper(l) == toupper(r);
    }

    char* host_strerror(int num) {
        switch (num) {
        case HOST_NOT_FOUND:
            return "The specified host is unknown";

        case NO_ADDRESS:
            return "The requested name is valid but does "
                "not have an IP address.";

        case NO_RECOVERY:
            return "A non-recoverable name server error occurred.";

        case TRY_AGAIN:
            return "A temporary error occurred on an authoritative "
                "name server.  Try again later.";

        default:
            return "Unknown DNS related error.";
        }
    }

    void resolveHost(const std::string &host, in_addr &addr) {
	struct hostent *he = gethostbyname(host.c_str());
        if (!he) {
            // oops
            throw HTTPError_t(HTTP_DNS, "Cannot resolve host '%s': <%d, %s>.",
                              host.c_str(), h_errno,
                              host_strerror(h_errno));
        }

        // remember IP address
        addr = *reinterpret_cast<in_addr*>(he->h_addr_list[0]);
    }
}

void URL_t::parse(const std::string &url) {
    // otestujeme url
    if ((url.length() < HTTP_SCHEMA.length()) ||
            (!equal(HTTP_SCHEMA.begin(), HTTP_SCHEMA.end(), url.begin(),
                    nocaseCmp)))
    {
        // oops, chyba -> zalogujeme
        throw TypeError_t("URL '%s' is not an HTTP URL.", url.c_str());
    }

    // najdeme pozici prvního lomítka
    std::string::size_type slash_pos = url.find('/', HTTP_SCHEMA.length());
    if (slash_pos != std::string::npos)
    {
        // pokud URL obsahuje lomítko, vezmene celou èást za lomítkem vèetnì
        // lomítka za cestu
        path = url.substr(slash_pos);
    }
    else
    {
        // jinak nastavíme cestu na root
        path = "/";
        slash_pos = url.size();
    }

    // najdeme první dvojteèku pøed lomítkem (oddìlovaè hostitele od portu)
    std::string::size_type colon_pos = url.find(':', HTTP_SCHEMA.length());
    if (colon_pos < slash_pos)
    {
        // pokud je dojteèka pøed lomítkem, rozdìlíme øetìzec na
        // hostitele a port
        host = url.substr(HTTP_SCHEMA.length(), colon_pos - HTTP_SCHEMA.length());
        ++colon_pos;
        // port pøevedeme na èíslo
        std::istrstream(url.substr(colon_pos, slash_pos - colon_pos).c_str()) >> port;
    }
    else
    {
        // celý øetìzec pøed lomítkem je hostitel
        host = url.substr(HTTP_SCHEMA.length(), slash_pos - HTTP_SCHEMA.length());
        // deault HTTP port
        port = 80;
    }
}

URL_t::URL_t(const std::string &url, const std::string &proxyVia)
    : usesProxy(!proxyVia.empty())
{
    if (!usesProxy) {
        // parse URL and resolve host to IP
        parse(url);
        resolveHost(host, addr);
        return;
    }

    // we have got proxy -> parse and resolve this URL
    parse(proxyVia);
    resolveHost(host, addr);

    // force path part to hold host URL
    path = url;
}

std::string URL_t::getUrl() const {
    // return just path part if we use proxy
    if (usesProxy) return path;

    std::ostringstream os;
    os << "http://" << host;
    if (port != 80) os  << ':' << port;
    os << path;
    return os.str();
}
