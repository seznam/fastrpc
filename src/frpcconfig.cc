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
 *
 *
 */

#include "frpcconfig.h"

#include <stdexcept>
#include <cstdlib>

namespace FRPC {

    LibConfig_t *LibConfig_t::m_instance = 0;

    LibConfig_t *LibConfig_t::getInstance() {

        if ( LibConfig_t::m_instance == 0 ) {
        const char *cfgFn = getenv("FASTRPC_CONFIG");
            if (cfgFn == 0) {
                static LibConfig_t config;
                m_instance = &config;
            } else {
                static LibConfig_t config(cfgFn);
                m_instance = &config;
            }
        }
        return LibConfig_t::m_instance;
    }

    LibConfig_t::LibConfig_t()
    : m_validateDatetime(true), m_validateString(false), m_preallocatedArraySize(4) {
    }

    LibConfig_t::LibConfig_t(const std::string &/*cfgFn*/) {
        throw std::runtime_error("Not yet implemented");
    }

    bool LibConfig_t::hasFeature(const std::string &/*feature*/) const {
        return false;
    }

    LibConfig_t::LibConfig_t(const LibConfig_t&) {
        // Intentionaly left empty.
    }

    LibConfig_t& LibConfig_t::operator=(const LibConfig_t&) {
        // Intentionaly left empty.
        return *this;
    }
}
