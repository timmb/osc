// Copyright (c) 2011 Toshiro Yamada
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/// @file tnyosc.hpp
/// @brief tnyosc main (and only) header file
/// @author Toshiro Yamada
///
/// tnyosc is a header-only Open Sound Control library written in C++ for
/// creating OSC-compliant messages. tnyosc supports Open Sound Control 1.0 and
/// 1.1 types and other nonstandard types, and bundles. Note that tnyosc does not
/// include code to actually send or receive OSC messages.

#pragma once

#include "utils.h"
#include "Message.h"

namespace osc {
	
/// This class represents an Open Sound Control bundle message. A bundle can
/// contain any number of Message and Bundle.
class Bundle : public TransportData {
public:
	
	//! Creates a OSC bundle with timestamp set to immediate. Call set_timetag to
	//! set a custom timestamp.
	Bundle();
	~Bundle() = default;
	
	//! Appends an OSC message to this bundle. The message is immediately copied
	//! into this bundle and any changes to the message after the call to this
	//! function does not affect this bundle.
	void append( const TransportData &message ) { append_data( message.byteArray() ); }
	
	/// Sets timestamp of the bundle.
	void set_timetag( uint64_t ntp_time );
	
	//! Returns a complete byte array of this OSC bundle type.
	const ByteBuffer& byteBuffer() const { return *mDataBuffer; }
	
	//! Returns the size of this OSC bundle.
	size_t size() const override { return mDataBuffer->size(); }
	
	//! Clears the bundle.
	void clear() { mDataBuffer->clear(); }
	
private:
	ByteBufferRef mDataBuffer;
	
	/// Returns a pointer to the byte array of this OSC bundle. This call is
	/// convenient for actually sending this OSC bundle.
	ByteBufferRef getSharedBuffer() const override;
	
	void append_data( const ByteBuffer& data );
};
	
}
