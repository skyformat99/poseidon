// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2015, LH_Mouse. All wrongs reserved.

#ifndef POSEIDON_HTTP_CLIENT_HPP_
#define POSEIDON_HTTP_CLIENT_HPP_

#include "../cxx_ver.hpp"
#include <string>
#include <cstddef>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include "request_headers.hpp"
#include "response_headers.hpp"
#include "../optional_map.hpp"
#include "../tcp_client_base.hpp"
#include "../stream_buffer.hpp"

namespace Poseidon {

namespace Http {
	class Client : public TcpClientBase {
	private:
		class HeaderJob;
		class RequestJob;

	private:
		enum State {
			S_FIRST_HEADER		= 0,
			S_HEADERS			= 1,
			S_END_OF_ENTITY		= 2,
			S_IDENTITY			= 3,
			S_CHUNK_HEADER		= 4,
			S_CHUNK_DATA		= 5,
			S_CHUNKED_TRAILER	= 6,
		};

	protected:
		enum {
			CONTENT_CHUNKED		= (boost::uint64_t)-1,
			CONTENT_TILL_EOF	= (boost::uint64_t)-2,
		};

	private:
		StreamBuffer m_received;

		bool m_expectingNewLine;
		boost::uint64_t m_sizeExpecting;
		State m_state;

		ResponseHeaders m_responseHeaders;
		StreamBuffer m_entity;

	public:
		explicit Client(const IpPort &addr, boost::uint64_t keepAliveTimeout, bool useSsl);
		~Client();

	private:
		void onReadAvail(const void *data, std::size_t size) FINAL;
		void onReadHup() NOEXCEPT OVERRIDE;

	protected:
		// 和 Http::Session 不同，这个函数在主线程中调用。
		// 如果 Transfer-Encoding 是 chunked， contentLength 的值为 CONTENT_CHUNKED。
		// 如果没有指定 Content-Length 同时也不是 chunked，contentLength 的值为 CONTENT_TILL_EOF。
		virtual void onHeader(const ResponseHeaders &responseHeaders, boost::uint64_t contentLength) = 0;
		// 报文可能分几次收到。
		virtual void onResponse(boost::uint64_t contentOffset, const StreamBuffer &entity) = 0;

	public:
		bool send(RequestHeaders requestHeaders, StreamBuffer entity = VAL_INIT, bool fin = false);

		// 需要预先对 URI 进行编码处理。
		bool send(Verb verb, std::string uri, OptionalMap getParams = VAL_INIT, OptionalMap headers = VAL_INIT,
			StreamBuffer entity = VAL_INIT, bool fin = false);
		bool send(Verb verb, std::string uri, OptionalMap getParams, StreamBuffer entity, bool fin = false){
			return send(verb, STD_MOVE(uri), STD_MOVE(getParams), OptionalMap(), STD_MOVE(entity), fin);
		}
	};
}

}

#endif
