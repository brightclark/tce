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
 * @file TPEFSectionReader.cc
 *
 * Definitions of TPEFSectionReader class.
 *
 * @author Mikael Lepist� 2003 (tmlepist-no.spam-cs.tut.fi)
 *
 * @note rating: yellow
 */

#include "TPEFSectionReader.hh"

namespace TPEF {

TPEFSectionReader::Header TPEFSectionReader::header_;

using ReferenceManager::SectionKey;
using ReferenceManager::SectionIndexKey;
using ReferenceManager::SectionOffsetKey;
using ReferenceManager::SafePointer;

/**
 * Constructor
 */
TPEFSectionReader::TPEFSectionReader() : SectionReader() {
}

/**
 * Destructor
 */
TPEFSectionReader::~TPEFSectionReader() {
}

/**
 * Returns binary reader instance which uses TPEFSectionReader classes.
 *
 * @return Binary reader which uses TPEFSectionReader classes.
 */
BinaryReader*
TPEFSectionReader::parent() const {
    return TPEFReader::instance();
}

/**
 * Reads section data from TPEF binary file.
 *
 * Base implementation for all TPEF section readers. Reads and keeps section
 * header data for later use by concrete section readers.  Header data is
 * accessed by readData methods of subclasses.
 *
 * @param stream Stream to be read from.
 * @param section Section where the information is to be stored.
 * @exception UnreachableStream If reading of section fails.
 * @exception KeyAlreadyExists Key was in use when trying to register object.
 * @exception EndOfFile If end of file were reached while it shouldn't.
 * @exception OutOfRange Some of read values were out of range.
 * @exception WrongSubclass Some class couldn't do what it was asked for.
 * @exception UnexpectedValue If there was unexpected value when reading.
 */
void
TPEFSectionReader::readData(
    BinaryStream& stream,
    Section* section) const
    throw (UnreachableStream, KeyAlreadyExists, EndOfFile,
           OutOfRange, WrongSubclass, UnexpectedValue) {

    TPEFReader* tpefReader = dynamic_cast<TPEFReader*>(parent());

    FileOffset startOffset = stream.readPosition();
    stream.setReadPosition(startOffset + TPEFHeaders::SH_FLAGS);
    Byte sectionFlags = stream.readByte();

    // if section vLen flag is not equal to the read value
    if(section->vLen() != ((sectionFlags & Section::SF_VLEN) != 0)) {
        throw UnexpectedValue(
            __FILE__, __LINE__, __func__,
            "read SF_VLEN flag doesn't match for section");
    }

    section->setFlags(sectionFlags);

    stream.setReadPosition(startOffset + TPEFHeaders::SH_ADDR);
    section->setStartingAddress(stream.readWord());

    stream.setReadPosition(startOffset + TPEFHeaders::SH_LINK);
    header_.linkId = stream.readHalfWord();

    SectionKey linkKey(header_.linkId);
    section->setLink(CREATE_SAFEPOINTER(linkKey));

    // set address space reference
    stream.setReadPosition(startOffset + TPEFHeaders::SH_ASPACE);
    Byte aSpaceIndex = stream.readByte();
    SectionIndexKey aSpaceKey(tpefReader->aSpaceId(), aSpaceIndex);
    section->setASpace(CREATE_SAFEPOINTER(aSpaceKey));

    // set name
    stream.setReadPosition(startOffset + TPEFHeaders::SH_NAME);
    Word sectionOffsetOfName = stream.readWord();

    if (tpefReader->strTableId() != 0) {
        SectionOffsetKey nameKey(tpefReader->strTableId(),
                                 sectionOffsetOfName);

        section->setName(CREATE_SAFEPOINTER(nameKey));
    } else {
        section->setName(&SafePointer::null);
    }

    // add section key for new section
    stream.setReadPosition(startOffset + TPEFHeaders::SH_ID);
    header_.sectionId = stream.readHalfWord();

    SectionKey sectionKey(header_.sectionId);
    SafePointer::addObjectReference(sectionKey, section);

    stream.setReadPosition(startOffset + TPEFHeaders::SH_INFO);
    readInfo(stream, section);

    stream.setReadPosition(startOffset + TPEFHeaders::SH_ENTSIZE);
    header_.elementSize = stream.readWord();

    // read offset of section body
    stream.setReadPosition(startOffset + TPEFHeaders::SH_OFFSET);
    header_.bodyOffset = stream.readWord();

    stream.setReadPosition(startOffset + TPEFHeaders::SH_SIZE);
    header_.bodyLength = stream.readWord();

    // actual class does reading of section body
    stream.setReadPosition(header_.bodyOffset);
}

/**
 * Reads info field of section header.
 *
 * Read position of stream will be moved 4 bytes forward.
 *
 * @param stream Stream where from info word is read.
 */
void
TPEFSectionReader::readInfo(BinaryStream& stream,
                            Section* /*sect*/) const {
    // move four bytes forward by default
    stream.readWord();
}

/**
 * Returns headers of section that we are currently reading.
 *
 * @return Headers of section that we are currently reading.
 */
const TPEFSectionReader::Header&
TPEFSectionReader::header() {
    return header_;
}

}
