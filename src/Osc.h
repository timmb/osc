/*
 Copyright (c) 2015, The Cinder Project, All rights reserved.
 
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

/*
 Toshiro Yamada
 Portions Copyright (c) 2011
 Distributed under the BSD-License.
 https://github.com/toshiroyamada/tnyosc
 */

#pragma once
#if ! defined( ASIO_STANDALONE )
#define ASIO_STANDALONE
#endif
#include "asio/asio.hpp"

#include <mutex>

#include "cinder/Buffer.h"
#include "cinder/app/App.h"

#if ! defined( _MSC_VER ) || _MSC_VER >= 1900
#define NOEXCEPT noexcept
#else 
#define NOEXCEPT
#endif

namespace osc {
	
//! Argument types suported by the Message class
enum class ArgType { INTEGER_32, FLOAT, DOUBLE, STRING, BLOB, MIDI, TIME_TAG, INTEGER_64, BOOL_T, BOOL_F, CHAR, NULL_T, INFINITUM, NONE };
	
// Forward declarations
class Message;
using UdpSocketRef = std::shared_ptr<asio::ip::udp::socket>;
using TcpSocketRef = std::shared_ptr<asio::ip::tcp::socket>;
using AcceptorRef = std::shared_ptr<asio::ip::tcp::acceptor>;
	
template<size_t size>
using ByteArray = std::array<uint8_t, size>;
using ByteBuffer = std::vector<uint8_t>;
using ByteBufferRef = std::shared_ptr<ByteBuffer>;
	
/// This class represents an Open Sound Control message. It supports Open Sound
/// Control 1.0 and 1.1 specifications and extra non-standard arguments listed
/// in http://opensoundcontrol.org/spec-1_0.
class Message {
public:
	
	//! Create an OSC message.
	Message() = default;
	explicit Message( const std::string& address );
	Message( const Message & ) = delete;
	Message& operator=( const Message & ) = delete;
	Message( Message && ) NOEXCEPT;
	Message& operator=( Message && ) NOEXCEPT;
	template <typename... Args>
	Message(const std::string& address, Args&&... args)
		: Message(address)
	{
		appendArgs(std::forward<Args>(args)...);
	}
	~Message() = default;
	
	// Functions for appending OSC 1.0 types
	
	//! Appends an int32 to the back of the message.
	void append( int32_t v );
	//! Appends a float to the back of the message.
	void append( float v );
	//! Appends a string to the back of the message.
	void append( const std::string& v );
	//! Appends an osc blob to the back of the message.
	void appendBlob( void* blob, uint32_t size );
	//! Appends an osc blob to the back of the message.
	void appendBlob( const ci::Buffer &buffer );
	
	// Functions for appending OSC 1.1 types
	
	//! Appends an OSC-timetag (NTP format) to the back of the message.
	void appendTimeTag( uint64_t v );
	//! Appends the current UTP timestamp to the back of the message.
	void appendCurrentTime();
	//! Appends a 'T'(True) or 'F'(False) to the back of the message.
	void append( bool v );
	//! Appends a Null (or nil) to the back of the message.
	void appendNull() { mIsCached = false; mDataViews.emplace_back( this, ArgType::NULL_T, -1, 0 ); }
	//! Appends an Impulse (or Infinitum) to the back of the message
	void appendImpulse() { mIsCached = false; mDataViews.emplace_back( this, ArgType::INFINITUM, -1, 0 ); }
	
	// Functions for appending nonstandard types
	
	//! Appends an int64_t to the back of the message.
	void append( int64_t v );
	//! Appends a float64 (or double) to the back of the message.
	void append( double v );
	//! Appends an ascii character to the back of the message.
	void append( char v );
	//! Appends a midi value to the back of the message.
	void appendMidi( uint8_t port, uint8_t status, uint8_t data1, uint8_t data2 );
	// TODO: figure out if array is useful.
	// void appendArray( void* array, size_t size );
    
    // Variadic template append function to add multiple args of arbitrary type
    template <typename T, typename... Args>
    void appendArgs(T&& t, Args&&... args)
    {
        appendArgs(std::forward<T>(t));
        appendArgs(std::forward<Args>(args)...);
    }
    
    template <typename T>
    void appendArgs(T&& t)
    {
        append(std::forward<T>(t));
    }
	
	//! Returns the int32_t located at \a index. If index is out of bounds, throws ExcIndexOutOfBounds.
	//! If argument isn't convertible to this type, throws ExcNonConvertible
	int32_t		getArgInt( uint32_t index ) const;
	//! Returns the float located at \a index. If index is out of bounds, throws ExcIndexOutOfBounds.
	//! If argument isn't convertible to this type, throws ExcNonConvertible
	float		getArgFloat( uint32_t index ) const;
	//! Returns the string located at \a index. If index is out of bounds, throws ExcIndexOutOfBounds.
	//! If argument isn't convertible to this type, throws ExcNonConvertible
	std::string getArgString( uint32_t index ) const;
	//! Returns the time_tag located at \a index. If index is out of bounds, throws ExcIndexOutOfBounds.
	//! If argument isn't convertible to this type, throws ExcNonConvertible
	int64_t		getArgTime( uint32_t index ) const;
	//! Returns the time_tag located at \a index. If index is out of bounds, throws ExcIndexOutOfBounds.
	//! If argument isn't convertible to this type, throws ExcNonConvertible
	int64_t		getArgInt64( uint32_t index ) const;
	//! Returns the double located at \a index. If index is out of bounds, throws ExcIndexOutOfBounds.
	//! If argument isn't convertible to this type, throws ExcNonConvertible
	double		getArgDouble( uint32_t index ) const;
	//! Returns the bool located at \a index. If index is out of bounds, throws ExcIndexOutOfBounds.
	//! If argument isn't convertible to this type, throws ExcNonConvertible
	bool		getArgBool( uint32_t index ) const;
	//! Returns the char located at \a index. If index is out of bounds, throws ExcIndexOutOfBounds.
	//! If argument isn't convertible to this type, throws ExcNonConvertible
	char		getArgChar( uint32_t index ) const;
	//! Supplies values for the four arguments in the midi format located at \a index. If index is out
	//! of bounds, throws ExcIndexOutOfBounds. If argument isn't convertible to this type, throws
	//! ExcNonConvertible
	void		getArgMidi( uint32_t index, uint8_t *port, uint8_t *status, uint8_t *data1, uint8_t *data2 ) const;
	//! Returns the blob, as a ci::Buffer, located at \a index. If index is out of bounds, throws
	//! ExcIndexOutOfBounds. If argument isn't convertible to this type, throws ExcNonConvertible
	ci::Buffer	getArgBlob( uint32_t index ) const;
	//! Supplies the blob data located at \a index to the \a dataPtr and \a size. Note: Doesn't copy.
	//! If index is out of bounds, throws ExcIndexOutOfBounds. If argument isn't convertible to this type,
	//! throws ExcNonConvertible
	void getArgBlobData( uint32_t index, const void **dataPtr, size_t *size ) const;
	
	//! Returns the argument type located at \a index.
	ArgType		getArgType( uint32_t index ) const;
	
	//! Sets the OSC address of this message.
	void setAddress( const std::string& address );
	//! Returns the OSC address of this message.
	const std::string& getAddress() const { return mAddress; }
	
	//! Returns the size of this OSC message as a complete packet.
	//! Performs a lazy cache
	size_t size() const;
	/// Clears the message, specifically any cache, dataViews, and address.
	void clear();
	
	class Argument {
	public:
		Argument();
		Argument( Message *owner, ArgType type, int32_t offset, uint32_t size, bool needsSwap = false );
		Argument( const Argument &arg ) = delete;
		Argument& operator=( const Argument &arg ) = delete;
		Argument( Argument &&arg ) NOEXCEPT;
		Argument& operator=( Argument &&arg ) NOEXCEPT;
		
		~Argument() = default;
		
		//! Returns the arguments type as an ArgType
		ArgType		getType() const { return mType; }
		//! Returns the arguments size, without trailing zeros or size int's taken into account.
		uint32_t	getSize() const { return mSize; }
		//! Returns the offset into the dataBuffer, where this Argument starts.
		int32_t		getOffset() const { return mOffset; }
		
		//! returns the underlying argument as an int32. If argument isn't convertible to this type,
		//! throws ExcNonConvertible
		int32_t		int32() const;
		//! returns the underlying argument as an int64. If argument isn't convertible to this type,
		//! throws ExcNonConvertible
		int64_t		int64() const;
		//! returns the underlying argument as a float. If argument isn't convertible to this type,
		//! throws ExcNonConvertible
		float		flt() const;
		//! returns the underlying argument as a double. If argument isn't convertible to this type,
		//! throws ExcNonConvertible
		double		dbl() const;
		//! returns the underlying argument as a boolean. If argument isn't convertible to this type,
		//! throws ExcNonConvertible
		bool		boolean() const;
		//! Supplies values for the four arguments in the midi format located at \a index. If argument
		//! isn't convertible to this type, throws ExcNonConvertible.
		//! ExcNonConvertible
		void		midi( uint8_t *port, uint8_t *status, uint8_t *data1, uint8_t *data2 ) const;
		//! Returns the underlying argument as a "deep-copied" ci::Buffer. If argument isn't convertible
		//! to this type, throws ExcNonConvertible
		ci::Buffer	blob() const;
		//! Supplies the blob data located at \a index to the \a dataPtr and \a size. Note: Doesn't copy.
		//! If argument isn't convertible to this type, throws ExcNonConvertible
		void		blobData( const void **dataPtr, size_t *size ) const;
		//! Returns the underlying argument as a char. If argument isn't convertible to this type,
		//! throws ExcNonConvertible
		char		character() const;
		//! Returns the underlying argument as a string. If argument isn't convertible to this type,
		//! throws ExcNonConvertible
		std::string string() const;
		
		//! Evaluates the equality of this with \a other
		bool operator==( const Argument &other ) const;

	private:
		//! Simple helper to stream a message's contents to the console.
		void		outputValueToStream( std::ostream &ostream ) const;
		//! Returns true, if before transporting this message, this argument needs a big endian swap
		bool		needsEndianSwapForTransmit() const { return mNeedsEndianSwapForTransmit; }
		//! Implements the swap for all types needing a swap.
		void		swapEndianForTransmit( uint8_t* buffer ) const;
		//! Helper to check if the underlying type is able to be converted to the provided template
		//! type \a T.
		template<typename T>
		bool convertible() const;
		//! Helper function to translate from ArgType to the spec's representation of that type.
		static char translateArgTypeToChar( ArgType type );
		//! Helper function to translate from the spec's character representation to the ArgType.
		static ArgType translateCharToArgType( char type );
		
		Message*		mOwner;
		ArgType			mType;
		int32_t			mOffset;
		uint32_t		mSize;
		bool			mNeedsEndianSwapForTransmit;
		
		friend class Message;
		friend std::ostream& operator<<( std::ostream &os, const Message &rhs );
	};
	//! Subscript operator returns a const Argument& based on \a index. If index is out of
	//! bounds, throws ExcIndexOutOfBounds.
	const Argument& operator[]( uint32_t index ) const;
	//! Evaluates the equality of this with \a other
	bool			operator==( const Message &other ) const;
	//! Evaluates the inequality of this with \a other
	bool			operator!=( const Message &other ) const;
	
private:
	//! Helper to calculate how many zeros to buffer to create a 4 byte
	static uint8_t getTrailingZeros( size_t bufferSize ) { return 4 - ( bufferSize % 4 ); }
	//! Helper to get current offset into the buffer.
	size_t getCurrentOffset() { return mDataBuffer.size(); }
	
	template<typename T>
	const Argument& getDataView( uint32_t index ) const;
	
	//! Helper to to insert data starting at \a begin for \a with resize/fill in the amount
	//! of \a trailingZeros
	void appendDataBuffer( const void *begin, uint32_t size, uint32_t trailingZeros = 0 );
	
	//! Returns a complete byte array of this OSC message as a ByteBufferRef type.
	//! The byte buffer is constructed lazily and is cached until the cache is
	//! obsolete. Call to |data| and |size| perform the same caching.
	ByteBufferRef getSharedBuffer() const;
	
	std::string				mAddress;
	ByteBuffer				mDataBuffer;
	std::vector<Argument>	mDataViews;
	mutable bool			mIsCached;
	mutable ByteBufferRef	mCache;
	
	//! Create the OSC message and store it in cache.
	void createCache() const;
	//! Used by receiver to create the inner message.
	bool bufferCache( uint8_t *data, size_t size );
	
	friend class Bundle;
	friend class SenderBase;
	friend class SenderUdp;
	friend class ReceiverBase;
	friend std::ostream& operator<<( std::ostream &os, const Message &rhs );
};

//! Convenient stream operator for Message
std::ostream& operator<<( std::ostream &os, const Message &rhs );
	
//! Represents an Open Sound Control bundle message. A bundle can contains any number
//! of Messages and Bundles.
class Bundle {
public:
	
	//! Creates a OSC bundle with timestamp set to immediate. Call set_timetag to
	//! set a custom timestamp.
	Bundle();
	~Bundle() = default;
	
	//! Appends an OSC message to this bundle. The message's byte buffer is immediately
	//! copied into this bundle and any changes to the message after the call to this
	//! function does not affect this bundle.
	void append( const Message &message ) { appendData( message.getSharedBuffer() ); }
	//! Appends an OSC bundle to this bundle. The bundle's contents are immediately copied
	//! into this bundle and any changes to the message after the call to this
	//! function does not affect this bundle.
	void append( const Bundle &bundle ) { appendData( bundle.getSharedBuffer() ); }
	
	/// Sets timestamp of the bundle.
	void setTimetag( uint64_t ntp_time );
	
	//! Returns the size of this OSC bundle.
	size_t size() const { return mDataBuffer->size(); }
	
	//! Clears the bundle.
	void clear() { mDataBuffer->clear(); }
	
private:
	ByteBufferRef mDataBuffer;
	
	/// Returns a pointer to the byte array of this OSC bundle. This call is
	/// convenient for actually sending this OSC bundle.
	ByteBufferRef getSharedBuffer() const;
	
	void appendData( const ByteBufferRef& data );
	
	friend class SenderBase;
	friend class SenderUdp;
};

//! Represents an OSC Sender (called a \a server in the OSC spec) and implements a unified
//! interface without implementing any of the networking layer.
class SenderBase {
public:
	//! Alias function that represents a general error callback for the socket. Note: for some errors
	//! there'll not be an accompanying oscAddress, or it'll not have a value set. These errors have
	//! nothing to do with transport but other socket operations like bind and open. To see more about
	//! asio's error_codes, look at "asio/error.hpp".
	using SocketTransportErrorFn = std::function<void( const asio::error_code & /*error*/,
													   const std::string & /*oscAddress*/)>;
	
	//! Binds the underlying network socket. Should be called before trying any communication operations.
	void bind() { bindImpl(); }
	//! Sends \a message to the destination endpoint.
	void send( const Message &message ) { sendImpl( message.getSharedBuffer() ); }
	//! Sends \a bundle to the destination endpoint.
	void send( const Bundle &bundle ) { sendImpl( bundle.getSharedBuffer() ); }
	//! Closes the underlying connection to the socket.
	void close() { closeImpl(); }
	
	//! Sets the underlying socket transport error fn with \a errorFn.
	void setSocketTransportErrorFn( SocketTransportErrorFn errorFn );
	
protected:
	SenderBase() = default;
	virtual ~SenderBase() = default;
	SenderBase( const SenderBase &other ) = delete;
	SenderBase& operator=( const SenderBase &other ) = delete;
	SenderBase( SenderBase &&other ) = delete;
	SenderBase& operator=( SenderBase &&other ) = delete;
	
	//! Abstract send function implemented by the network layer.
	virtual void sendImpl( const ByteBufferRef &byteBuffer ) = 0;
	//! Abstract close function implemented by the network layer
	virtual void closeImpl() = 0;
	//! Abstract bind function implemented by the network layer
	virtual void bindImpl() = 0;
	
	SocketTransportErrorFn	mSocketTransportErrorFn;
	std::mutex				mSocketErrorFnMutex;
};
	
//! Represents an OSC Sender (called a \a server in the OSC spec) and implements the UDP
//! transport networking layer.
class SenderUdp : public SenderBase {
public:
	//! Alias protocol for cleaner interfaces
	using protocol = asio::ip::udp;
	//! Constructs a Sender (called a \a server in the OSC spec) using UDP as transport, whose local endpoint is
	//! defined by \a localPort and \a protocol, which defaults to v4, and remote endpoint is defined by
	//! \a destinationHost and \a destinationPort. Takes an optional io_service to construct the socket from.
	SenderUdp( uint16_t localPort,
			   const std::string &destinationHost,
			   uint16_t destinationPort,
			   const protocol &protocol = protocol::v4(),
			   asio::io_service &service = ci::app::App::get()->io_service() );
	//! Constructs a Sender (called a \a server in the OSC spec) using UDP as transport, whose local endpoint is
	//! defined by \a localPort and \a protocol, which defaults to v4, and remote endpoint is defined by \a
	//! destination. Takes an optional io_service to construct the socket from.
	SenderUdp( uint16_t localPort,
			   const protocol::endpoint &destination,
			   const protocol &protocol = protocol::v4(),
			   asio::io_service &service = ci::app::App::get()->io_service() );
	//! Constructs a Sender (called a \a server in the OSC spec) using UDP for transport, with an already created
	//! udp::socket shared_ptr \a socket and remote endpoint \a destination. This constructor is good for using
	//! already constructed sockets for more indepth configuration. Expects the local endpoint to be constructed.
	SenderUdp( const UdpSocketRef &socket, const protocol::endpoint &destination );
	//! Default virtual constructor
	virtual ~SenderUdp() = default;
	
	//! Returns the local address of the endpoint associated with this socket.
	protocol::endpoint getLocalAddress() const { return mSocket->local_endpoint(); }
	//! Returns the remote address of the endpoint associated with this transport.
	const protocol::endpoint& getRemoteAddress() const { return mRemoteEndpoint; }
	
protected:
	//! Opens and Binds the underlying UDP socket to the protocol and localEndpoint respectively.
	void bindImpl() override;
	//! Sends the byte buffer /a data to the remote endpoint using the UDP socket, asynchronously.
	void sendImpl( const ByteBufferRef &data ) override;
	//! Closes the underlying UDP socket.
	void closeImpl() override;
	
	UdpSocketRef			mSocket;
	protocol::endpoint		mLocalEndpoint, mRemoteEndpoint;
	
public:
	//! Non-copyable.
	SenderUdp( const SenderUdp &other ) = delete;
	//! Non-copyable.
	SenderUdp& operator=( const SenderUdp &other ) = delete;
	//! Non-Moveable.
	SenderUdp( SenderUdp &&other ) = delete;
	//! Non-Moveable.
	SenderUdp& operator=( SenderUdp &&other ) = delete;
};

//! Represents an OSC Sender (called a \a server in the OSC spec) and implements the TCP
//! transport networking layer.
class SenderTcp : public SenderBase {
public:
	using protocol = asio::ip::tcp;
	//! Constructs a Sender (called a \a server in the OSC spec) using TCP as transport, whose local endpoint is
	//! defined by \a localPort and \a protocol, which defaults to v4, and remote endpoint is defined by \a
	//! destinationHost and \a destinationPort. Takes an optional io_service to construct the socket from.
	SenderTcp( uint16_t localPort,
			   const std::string &destinationHost,
			   uint16_t destinationPort,
			   const protocol &protocol = protocol::v4(),
			   asio::io_service &service = ci::app::App::get()->io_service() );
	//! Constructs a Sender (called a \a server in the OSC spec) using TCP as transport, whose local endpoint is
	//! defined by \a localPort and \a protocol, which defaults to v4, and remote endpoint is defined by \a
	//! destination. Takes an optional io_service to construct the socket from.
	SenderTcp( uint16_t localPort,
			   const protocol::endpoint &destination,
			   const protocol &protocol = protocol::v4(),
			   asio::io_service &service = ci::app::App::get()->io_service() );
	//! Constructs a Sender (called a \a server in the OSC spec) using TCP as transport, with an already created
	//! tcp::socket shared_ptr \a socket and remote endpoint \a destination. This constructor is good for using
	//! already constructed sockets for more indepth configuration. Expects the local endpoint is already
	//! constructed.
	SenderTcp( const TcpSocketRef &socket, const protocol::endpoint &destination );
	virtual ~SenderTcp() = default;
	
	//! Connects to the remote endpoint using the underlying socket. Has to be called before attempting to send anything.
	void connect();
	
	//! Returns the local address of the endpoint associated with this socket.
	protocol::endpoint getLocalEndpoint() const { return mSocket->local_endpoint(); }
	//! Returns the remote address of the endpoint associated with this transport.
	const protocol::endpoint& getRemoteEndpoint() const { return mRemoteEndpoint; }
	
protected:
	//! Opens and Binds the underlying TCP socket to the protocol and localEndpoint respectively.
	void bindImpl() override;
	//! Sends the byte buffer /a data to the remote endpoint using the TCP socket, asynchronously.
	void sendImpl( const ByteBufferRef &data ) override;
	//! Closes the underlying TCP socket.
	void closeImpl() override { mSocket->close(); }
	
	TcpSocketRef			mSocket;
	asio::ip::tcp::endpoint mLocalEndpoint, mRemoteEndpoint;
	
public:
	//! Non-copyable.
	SenderTcp( const SenderTcp &other ) = delete;
	//! Non-copyable.
	SenderTcp& operator=( const SenderTcp &other ) = delete;
	//! Non-Moveable.
	SenderTcp( SenderTcp &&other ) = delete;
	//! Non-Moveable.
	SenderTcp& operator=( SenderTcp &&other ) = delete;
};

//! Represents an OSC Receiver(called a \a client in the OSC spec) and implements a unified
//! interface without implementing any of the networking layer.
class ReceiverBase {
public:
	//! Alias function that represents a transport error function. To see more about
	//! asio's error_codes, look at "asio/error.hpp".
	template<typename Protocol>
	using SocketTransportErrorFn = std::function<void( const asio::error_code &/*error*/,
												   const typename Protocol::endpoint &/*originator*/)>;
	//! Alias function representing a message callback.
	using ListenerFn = std::function<void( const Message &message )>;
	//! Alias container for callbacks.
	using Listeners = std::vector<std::pair<std::string, ListenerFn>>;
	
	//! Binds the underlying network socket. Should be called before trying communication operations.
	void		bind() { bindImpl(); }
	//! Commits the socket to asynchronously listen and begin to receive from outside connections.
	void		listen() { listenImpl(); }
	//! Closes the underlying network socket. Should be called on most errors to reset the socket.
	void		close() { closeImpl(); }
	
	//! Sets a callback, \a listener, to be called when receiving a message with \a address. If a listener exists for this address, \a listener will replace it.
	void		setListener( const std::string &address, ListenerFn listener );
	//! Removes the listener associated with \a address.
	void		removeListener( const std::string &address );
	
protected:
	ReceiverBase() = default;
	virtual ~ReceiverBase() = default;
	//! Non-copyable.
	ReceiverBase( const ReceiverBase &other ) = delete;
	//! Non-copyable.
	ReceiverBase& operator=( const ReceiverBase &other ) = delete;
	//! Non-Moveable.
	ReceiverBase( ReceiverBase &&other ) = delete;
	//! Non-Moveable.
	ReceiverBase& operator=( ReceiverBase &&other ) = delete;
	
	//! decodes and routes messages from the networking layers stream.
	void dispatchMethods( uint8_t *data, uint32_t size );
	
	//! Decodes a complete OSC Packet into it's individual parts.
	bool decodeData( uint8_t *data, uint32_t size, std::vector<Message> &messages, uint64_t timetag = 0 ) const;
	//! Decodes an individual message.
	bool decodeMessage( uint8_t *data, uint32_t size, std::vector<Message> &messages, uint64_t timetag = 0 ) const;
	//! Matches the addresses of messages based on the OSC spec.
	bool patternMatch( const std::string &lhs, const std::string &rhs ) const;
	
	//! Abstract bind implementation function.
	virtual void bindImpl() = 0;
	//! Abstract listen implementation function.
	virtual void listenImpl() = 0;
	//! Abstract close implementation function.
	virtual void closeImpl() = 0;
	
	Listeners				mListeners;
	std::mutex				mListenerMutex, mSocketTransportErrorFnMutex;
};
	
//! Represents an OSC Receiver(called a \a client in the OSC spec) and implements the UDP transport
//!	networking layer.
class ReceiverUdp : public ReceiverBase {
public:
	using protocol = asio::ip::udp;
	//! Constructs a Receiver (called a \a client in the OSC spec) using UDP for transport, whose local endpoint
	//! is defined by \a localPort and \a protocol, which defaults to v4. Takes an optional io_service to
	//! construct the socket from.
	ReceiverUdp( uint16_t port,
				 const protocol &protocol = protocol::v4(),
				 asio::io_service &io = ci::app::App::get()->io_service()  );
	//! Constructs a Receiver (called a \a client in the OSC spec) using UDP for transport, whose local endpoint
	//! is defined by \a localEndpoint. Takes an optional io_service to construct the socket from.
	ReceiverUdp( const protocol::endpoint &localEndpoint,
				 asio::io_service &io = ci::app::App::get()->io_service() );
	//! Constructs a Receiver (called a \a client in the OSC spec) using UDP for transport, from the already
	//! constructed udp::socket shared_ptr \a socket. Use this for extra configuration and or sharing sockets
	//! between sender and receiver.
	ReceiverUdp( UdpSocketRef socket );
	
	virtual ~ReceiverUdp() = default;
	
	// TODO: Check to see that this is needed, see if we can't auto accept a size of datagram.
	void setAmountToReceive( uint32_t amountToReceive ) { mAmountToReceive = amountToReceive; }
	//! Returns the local udp::endpoint of the underlying socket.
	asio::ip::udp::endpoint getLocalEndpoint() { return mSocket->local_endpoint(); }
	
	//! Sets the underlying SocketTransportErrorFn based on the asio::ip::tcp protocol.
	void setSocketErrorFn( SocketTransportErrorFn<protocol> errorFn );
	
protected:
	//! Opens and Binds the underlying UDP socket to the protocol and localEndpoint respectively.
	void bindImpl() override;
	//! Listens on the UDP network socket for incoming datagrams. Handles the async receive completion operations. Any errors from asio are handled internally.
	void listenImpl() override;
	//! Closes the underlying UDP socket.
	void closeImpl() override { mSocket->close(); }
	
	UdpSocketRef						mSocket;
	asio::ip::udp::endpoint				mLocalEndpoint;
	asio::streambuf						mBuffer;
	
	SocketTransportErrorFn<protocol>	mSocketTransportErrorFn;
	
	uint32_t							mAmountToReceive;
	
public:
	//! Non-copyable.
	ReceiverUdp( const ReceiverUdp &other ) = delete;
	//! Non-copyable.
	ReceiverUdp& operator=( const ReceiverUdp &other ) = delete;
	//! Non-Moveable.
	ReceiverUdp( ReceiverUdp &&other ) = delete;
	//! Non-Moveable.
	ReceiverUdp& operator=( ReceiverUdp &&other ) = delete;
};

//! Represents an OSC Receiver(called a \a client in the OSC spec) and implements the TCP
//! transport networking layer.
class ReceiverTcp : public ReceiverBase {
public:
	using protocol = asio::ip::tcp;
	//! Constructs a Receiver (called a \a client in the OSC spec) using TCP for transport, whose local endpoint
	//! is defined by \a localPort and \a protocol, which defaults to v4. Takes an optional io_service to
	//! construct the socket from.
	ReceiverTcp( uint16_t port,
				 const protocol &protocol = protocol::v4(),
				 asio::io_service &service = ci::app::App::get()->io_service()  );
	//! Constructs a Receiver (called a \a client in the OSC spec) using TCP for transport, whose local endpoint
	//! is defined by \a localEndpoint. Takes an optional io_service to construct the socket from.
	ReceiverTcp( const protocol::endpoint &localEndpoint,
				 asio::io_service &service = ci::app::App::get()->io_service() );
	//! Constructs a Receiver (called a \a client in the OSC spec) using TCP for transport, from the already
	//! constructed tcp::acceptor shared_ptr \a socket. Use this for extra configuration.
	ReceiverTcp( AcceptorRef acceptor );
	virtual ~ReceiverTcp() = default;
	
	//! Sets the underlying SocketTransportErrorFn based on the asio::io::tcp protocol.
	void setSocketTransportErrorFn( SocketTransportErrorFn<protocol> errorFn );
	
protected:
	struct Connection {
		Connection( TcpSocketRef socket, ReceiverTcp* transport );
		
		~Connection();
		
		protocol::endpoint getRemoteEndpoint() { return mSocket->remote_endpoint(); }
		protocol::endpoint getLocalEndpoint() { return  mSocket->local_endpoint(); }
		
		//! Implements the read on the underlying socket. Handles the async receive completion operations. Any errors from asio are handled internally.
		void read();
		//! Simple alias for asio buffer iterator type.
		using iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
		//! Static method which is used to read the stream as it's coming in and notate each packet.
		static std::pair<iterator, bool> readMatchCondition( iterator begin, iterator end );
		//! Implements the close of this socket
		void close();
		
		TcpSocketRef			mSocket;
		ReceiverTcp*			mReceiver;
		asio::streambuf			mBuffer;
		std::vector<uint8_t>	mDataBuffer;
		
		//! Non-copyable.
		Connection( const Connection &other ) = delete;
		//! Non-copyable.
		Connection& operator=( const Connection &other ) = delete;
		//! Non-moveable.
		Connection( Connection &&other ) = delete;
		//! Non-moveable.
		Connection& operator=( Connection &&other ) = delete;
		
		friend class ReceiverTcp;
	};
	
	//! Opens and Binds the underlying TCP acceptor to the protocol and localEndpoint respectively.
	void bindImpl() override;
	//! Listens on the underlying TCP network socket for incoming connections.
	void listenImpl() override;
	//! Launches acceptor to asynchronously accept connections.
	void accept();
	//! Implements the close operation for the underlying sockets and acceptor.
	void closeImpl() override;
	
	AcceptorRef			mAcceptor;
	protocol::endpoint	mLocalEndpoint;
	
	SocketTransportErrorFn<protocol>	mSocketTransportErrorFn;
	
	std::mutex							mDispatchMutex, mConnectionMutex;
	
	using UniqueConnection = std::unique_ptr<Connection>;
	std::vector<UniqueConnection>			mConnections;

	friend struct Connection;
public:
	//! Non-copyable.
	ReceiverTcp( const ReceiverTcp &other ) = delete;
	//! Non-copyable.
	ReceiverTcp& operator=( const ReceiverTcp &other ) = delete;
	//! Non-Moveable.
	ReceiverTcp( ReceiverTcp &&other ) = delete;
	//! Non-Moveable.
	ReceiverTcp& operator=( ReceiverTcp &&other ) = delete;
};
	
//! Converts ArgType \a type to c-string for debug purposes.
const char* argTypeToString( ArgType type );

namespace time {
	using milliseconds = std::chrono::milliseconds;
	//! Returns system clock time.
	uint64_t get_current_ntp_time( milliseconds offsetMillis = milliseconds( 0 ) );
	//! Returns the current presentation time as NTP time, which may include an offset to the system clock.
	uint64_t getFutureClockWithOffset( milliseconds offsetFuture = milliseconds( 0 ), uint64_t localOffsetSecs = 0, uint64_t localOffsetUSecs = 0 );
	//! Returns the current presentation time as a string.
	std::string getClockString( uint64_t ntpTime, bool includeDate = false );
	//! Sets the current presentation time as NTP time, from which an offset to the system clock is calculated.
	void calcOffsetFromSystem( uint64_t ntpTime, uint64_t *localOffsetSecs, uint64_t *localOffsetUSecs );
};
	
class ExcIndexOutOfBounds : public ci::Exception {
public:
	ExcIndexOutOfBounds( const std::string &address, uint32_t index )
	: Exception( std::string( std::to_string( index ) + " out of bounds from address, " + address ) )
	{}
};

class ExcNonConvertible : public ci::Exception {
public:
	ExcNonConvertible( const std::string &address, ArgType actualType, ArgType convertToType )
	: Exception( address + ": expected type: " + argTypeToString( convertToType ) + ", actual type: " + argTypeToString( actualType ) )
	{}
};
	
}