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
 * FILE          $Id: frpcbinary.h,v 1.7 2008-05-05 12:52:00 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCBINARYREF_H
#define FRPCBINARYREF_H

#include <functional>

#include <frpcvalue.h>
#include <frpcpool.h>


#if __cplusplus >= 201103L

namespace FRPC {

/** The collection of two methods that provides access to list of chunks and
 * the size of final binary value.
 */
struct BinaryRefFeeder_t {
    struct Chunk_t {
        explicit operator bool() const { return data;}
        const uint8_t *data;
        std::size_t size;
    };
    std::function<std::size_t()> size;
    std::function<Chunk_t()> next;
};

/** Comparison operator for chunks.
 */
inline bool operator==(const BinaryRefFeeder_t::Chunk_t &lhs,
                       const BinaryRefFeeder_t::Chunk_t &rhs)
{
    return (lhs.data == rhs.data) && (lhs.size == rhs.size);
}

/** Binary type that does not hold any data.
 */
class FRPC_DLLEXPORT BinaryRef_t: public Value_t {
public:
    enum { TYPE = 0x0D };

    /** Container like object that provides iterator interface for chunk list.
     */
    struct Chunks_t {
        Chunks_t(BinaryRefFeeder_t feeder): feeder(feeder) {}

        /// The chunks iterator.
        struct const_iterator {
        public:
            using value_type      = BinaryRefFeeder_t::Chunk_t;
            using difference_type = std::ptrdiff_t;
            using pointer         = const BinaryRefFeeder_t::Chunk_t *;
            using reference       = const BinaryRefFeeder_t::Chunk_t &;

            const_iterator(const BinaryRefFeeder_t *feeder, value_type value)
                : feeder(feeder), value(value)
            {}

            const_iterator &operator++() { return incr(), *this;}
            const_iterator operator++(int) { auto t = *this; incr(); return t;}

            reference operator*() const { return value;}
            pointer operator->() const { return &value;}

            friend bool operator==(const const_iterator &lhs,
                                   const const_iterator &rhs)
            { return lhs.feeder == rhs.feeder && lhs.value == rhs.value;}
            friend bool operator!=(const const_iterator &lhs,
                                   const const_iterator &rhs)
            { return !(lhs == rhs);}

        private:
            void incr() { value = feeder->next();}

            const BinaryRefFeeder_t *feeder;
            value_type value;
        };

        const_iterator begin() const { return {&feeder, feeder.next()};}
        const_iterator end() const { return {&feeder, {nullptr, 0}};}

        BinaryRefFeeder_t feeder;
    };

    /** D'tor.
     */
    virtual ~BinaryRef_t() = default;

    /** Returns type of value.
     */
    virtual unsigned short getType() const { return TYPE;}

    /** Returns name of type.
     */
    virtual const char *getTypeName() const { return "binaryref";}

    /** Returns the size of final binary value.
     */
    std::size_t size() const { return feeder.size();}

    /** Returns container like objects that provides iterator interface for
     * chunk list.
     */
    Chunks_t chunks() const { return {feeder};}

    /** Returns the chunks feeder.
     */
    BinaryRefFeeder_t getFeeder() const { return feeder;}

    /** Returns new BinaryRef_t value.
     */
    virtual Value_t &clone(Pool_t &newPool) const {
        return newPool.BinaryRef(feeder);
    }

private:
    friend class Pool_t;

    /** Default constructor is disabled
     */
    BinaryRef_t() = delete;

    /** C'tor from feeder.
     */
    BinaryRef_t(BinaryRefFeeder_t feeder): feeder(feeder) {}

    BinaryRefFeeder_t feeder; //!< the data holder
};

/** Used to retype Value_t to BinaryRef_t.
 */
inline FRPC_DLLEXPORT BinaryRef_t &BinaryRef(Value_t &value) {
    if (auto binary = dynamic_cast<BinaryRef_t *>(&value)) return *binary;
    throw TypeError_t::format("Type is %s but not binaryref",
                              value.getTypeName());
}

/** Used to retype Value_t to BinaryRef_t.
 */
inline FRPC_DLLEXPORT const BinaryRef_t &BinaryRef(const Value_t &value) {
    if (auto binary = dynamic_cast<const BinaryRef_t *>(&value)) return *binary;
    throw TypeError_t::format("Type is %s but not binaryref",
                              value.getTypeName());
}

} // namespace FRPC

#endif /* __cplusplus >= 201103L */

#endif /* FRPCBINARYREF_H */
