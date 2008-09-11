/*
    Copyright 2002-2008 Tampere University of Technology.  All Rights
    Reserved.

    This file is part of TTA-Based Codesign Environment (TCE).

    TCE is free software; you can redistribute it and/or modify it under the
    terms of the GNU General Public License version 2 as published by the Free
    Software Foundation.

    TCE is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with TCE; if not, write to the Free Software Foundation, Inc., 51 Franklin
    St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/
/**
 * @file ProgramAnnotation.cc
 *
 * Implementation of ProgramAnnotation class.
 *
 * @author Pekka Jääskeläinen 2006 (pekka.jaaskelainen-no.spam-tut.fi)
 * @note rating: red
 */

#include "ProgramAnnotation.hh"

namespace TTAProgram {

/////////////////////////////////////////////////////////////////////////////
// ProgramAnnotation
/////////////////////////////////////////////////////////////////////////////

/**
 * Constructor.
 *
 * @param id The id of the annotation (a 24-bit value).
 * @param data The payload data as a string.
 */
ProgramAnnotation::ProgramAnnotation(Id id, const std::string& data) :
    id_(id) {
    payload_ = std::vector<Byte>(data.begin(), data.end());
}

/**
 * Constructor.
 *
 * @param id The id of the annotation (a 24-bit value).
 * @param data The payload data as a byte vector.
 */
ProgramAnnotation::ProgramAnnotation(
    Id id, const std::vector<Byte>& payload) :
    id_(id), payload_(payload) {
}

/**
 * Destructor.
 */
ProgramAnnotation::~ProgramAnnotation() {
}

/**
 * The payload data as a std::string.
 *
 * @returns The data as a std::string.
 */
std::string
ProgramAnnotation::stringValue() const {
    return std::string(payload_.begin(), payload_.end());
}

/**
 * Returns the id of the annotation.
 *
 * @return the id of the annotation.
 */
ProgramAnnotation::Id
ProgramAnnotation::id() const {
    return id_;
}

/**
 * Return the payload data as a non-mutable char vector.
 *
 * @return The payload data.
 */
const std::vector<Byte>&
ProgramAnnotation::payload() const {
    return payload_;
}

} // namespace TTAProgram
