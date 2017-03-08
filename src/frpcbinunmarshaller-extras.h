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
 * DESCRIPTION
 *
 * AUTHOR
 *              Jan Klesnil <jan.klesnil@firma.seznam.cz>
 */
#ifndef FRPCBINUNMARSHALLEREXTRAS_H
#define FRPCBINUNMARSHALLEREXTRAS_H

#include <frpcunmarshaller.h>

namespace FRPC {
namespace extras {

/**
 * @brief Returns the current depth of backtracing stack.
 * @param unmarshaller   An instance of binary unmarshaller, otherwise bad
 *                       dynamic cast exception is raised/thrown.
 */
size_t recursionDepth(const UnMarshaller_t *unmarshaller);

/**
 * @brief Parse only valid prefix of supplied data.
 *
 * In contrast to UnMarshaller_t::unMarshall() method this one does not throw
 * when illegal value type discriminator is encountered in given data.
 *
 * @param unmarshaller       An instance of binary unmarshaller, otherwise bad
 *                           dynamic cast exception is raised/thrown.
 * @param data               Binary data to parse.
 * @param size               Size of binary data to parse.
 * @param type               Same as the type parameter of unMarshall method
 *                           of UnMarshaller_t type.
 * @return                   Number of bytes that have been successfully parsed.
 */
size_t unMarshallPartial(UnMarshaller_t *unmarshaller,
                         const char *data, unsigned int size, char type);

/**
 * @brief Reset binary unmarshaller to fault state.
 *
 * Following data will be parsed as a fault structure.
 *
 * @param unmarshaller   An instance of binary unmarshaller, otherwise bad
 *                       dynamic cast exception is raised/thrown.
 */
void resetToFaultState(UnMarshaller_t *unmarshaller);

} // namespace extras
} // namespace FRPC

#endif // FRPCBINUNMARSHALLEREXTRAS_H
