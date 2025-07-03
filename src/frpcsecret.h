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
 * FILE          $Id: $
 *
 * DESCRIPTION
 *
 * AUTHOR Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2025
 * All Rights Reserved
 *
 */

#ifndef FRPCFRPCSECRET_H
#define FRPCFRPCSECRET_H

#include <frpcvalue.h>
#include <frpctypeerror.h>

namespace FRPC {

class Pool_t;

/** Wraps secret values, that should be handled with care.
 *
 * It is used to store sensitive data, like passwords, tokens, etc. Real value
 * is stored as nested FRPC value, which can be a string, binary, or any other
 * type.
*/
class FRPC_DLLEXPORT SecretValue_t: public Value_t {
public:
    enum {TYPE = TYPE_SECRET_VALUE};

    /** Returns a clone of the value in a new pool.
     */
    Value_t &clone(Pool_t &pool) const override;

    /** Returns the type of value.
     */
    TypeTag_t getType() const override {return TYPE;}

    /** Returns the name of the type.
     */
    const char *getTypeName() const override {return "secret_value";}

    /** Returns the actual value.
     */
    const Value_t &getValue() const {return *value;}

protected:
    friend class Pool_t;

    /** C'tor.
     */
    SecretValue_t(const Value_t *value): value(value) {}

    const Value_t *value; //!< pointer to the actual value with sensitive data
};

/** Convenience function to access SecretValue_t from Value_t.
 */
inline FRPC_DLLEXPORT SecretValue_t &SecretValue(Value_t &value) {
    if (auto *secret = dynamic_cast<SecretValue_t *>(&value))
        return *secret;
    throw TypeError_t::format(
        "Type is %s but not secret value",
        value.getTypeName()
    );
}

/** Convenience function to access SecretValue_t from Value_t.
 */
inline FRPC_DLLEXPORT const SecretValue_t &SecretValue(const Value_t &value) {
    if (const auto *secret = dynamic_cast<const SecretValue_t *>(&value))
        return *secret;
    throw TypeError_t::format(
        "Type is %s but not secret value",
        value.getTypeName()
    );
}

} // namespace FRPC

#endif /* FRPCFRPCSECRET_H */
