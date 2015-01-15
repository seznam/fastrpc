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

#ifndef __FRPC_CONFIG_H__
#define __FRPC_CONFIG_H__

#include <string>

namespace FRPC {

    /**
    * \brief FastRPC configuration object
    *
    * LibConfig_t is a singleton object that can be used to alter
    * default behaviour of certain library parts in runtime.
    * \warning LibConfig_t is meant to be used process wide. It is not
    *          thread safe and it does not use tls either. In multithreaded
    *          applications, setters should be used only once in 'main' process
    *          in order to avoid race conditions.
    **/
    class LibConfig_t {
        public:

            /**
            * \brief Returns instance of LibConfig_t
            **/
            static LibConfig_t *getInstance();

            /**
             * \brief Returns true if specific feature is present.
             *
             * This method is meant to be used to test if specific behaviour
             * of configuration object is present. Reserved for future use
             * \return true if given behaviour is present
            **/
            bool hasFeature(const std::string &feature) const;

            /**
             * \brief Returns Datetime_t validation policy
             * \see m_validateDatetime
            **/
            bool getDatetimeValidationPolicy() const {
                return m_validateDatetime;
            }

            /**
             * \brief Sets Datetime_t validation policy
             * \see m_validateDatetime
            **/
            void setDatetimeValidationPolicy(bool enabled) {
                m_validateDatetime = enabled;
            }

            /**
             * \brief Returns String_t validation policy
             * \see m_validateString
            **/
            bool getStringValidationPolicy() const {
                return m_validateString;
            }

            /**
             * \brief Sets String_t validation policy
             * \see m_validateString
            **/
            void setStringValidationPolicy(bool enabled) {
                m_validateString = enabled;
            }

            /**
             * \brief Returns number of preallocated Array_t members
             * \see m_preallocatedArraySize
            **/
            unsigned long getDefaultArraySize() const {
                return m_preallocatedArraySize;
            }

            /**
             * \brief Sets number of preallocated Array_t members
             * \see m_preallocatedArraySize
            **/
            void setDefaultArraySize(unsigned long size) {
                m_preallocatedArraySize = size;
            }

        protected:

            // TODO: Add IPv6/IPv4 resolution policies
            // TODO: Add SSL configuration

            /**
            * \brief Holds instance of LibConfig_t object
            **/
            static LibConfig_t *m_instance;

            /**
            * \brief Toggles Datetime_t validation
            *
            * If true (default), Datetime_t translates arguments using localtime_r
            * into local time.
            **/
            bool m_validateDatetime;

            /**
            * \brief Toggles String_t character validation
            *
            * If true, only characters from http://www.w3.org/TR/xml/#NT-Char
            * are allowed to be present and string must be in UTF-8.
            * If false (default), String_t accepts any character
            **/
            bool m_validateString;

            /**
            * \brief Sets Array_t's preallocated space
            *
            * Positive number, that specifies number of preallocated
            * members in Array_t, default 4
            **/
            unsigned long m_preallocatedArraySize;

            /**
            * \brief Default constructor
            *
            * Constructs config object using default values
            **/
            LibConfig_t();

            /**
            * \brief Parametric constructor
            *
            * Loads library configuration from given file
            * \warning Not yet implemented
            **/
            LibConfig_t(const std::string &cfgFn);

            //! No copies
            LibConfig_t(const LibConfig_t&);

            //! No assignemnts
            LibConfig_t& operator=(const LibConfig_t&);


    };

};

#endif
