#pragma once

#include "pch.h"
#include <vector>

namespace com {
	class port {

	public:
		typedef byte_t number;
		typedef void *handle;

		struct config {

			struct dcb_t {
				enum class baud_rate : unsigned {		// см. константы CBR_XXX в <winbase.h>
					cbr_115200 = 115200,
					cbr_128000 = 128000,
					cbr_256000 = 256000
				};
				baud_rate baud_rate;

				enum class byte_size : byte_t {
					bs_7 = 7,
					bs_8 = 8
				};
				byte_size byte_size;

				enum class parity : byte_t {
					no = 0, odd, even, mark, space
				};
				parity parity;

				enum class stop_bits : byte_t {
					one = 0, one_and_a_half, two
				};
				stop_bits stop_bits;
			};

			dcb_t dcb;
			unsigned buffer_size;

			config(_in const dcb_t &dcb, _in unsigned buffer_size = 4096);
		};

	public:
		static std::vector<number> static__enum();

		port();
		~port();
		set_lasterror(bool) open(_in number number, _in const config &config);
		set_lasterror(bool) close();

	protected:
		struct buffer {
			typedef char char_t;
			typedef unsigned size_t;

			std::unique_ptr<char_t> data;
			size_t size = 0;
		};

	protected:
		static set_lasterror(buffer::size_t) static_send(_in handle handle, _in const buffer &buffer);		// bytes sended (not more then buffer.size)
		static set_lasterror(buffer::size_t) static_recieve(_in handle handle, _out buffer &buffer);		// bytes recieved (not more then buffer.size)
		static set_lasterror(handle) static__open(_in number number, _in const config::dcb_t &config_dcb);	// not nullptr on success, nullptr on error
		static set_lasterror(handle) static__close(_in handle handle);										// nullptr on success, same handle on error

	private:
		handle _handle;
		buffer _buffer;
	};
}