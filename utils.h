#pragma once

#include "pch.h"
#include <vector>

namespace string {
	//enum class case_sensitivity {
	//	no = 0,	yes
	//};

	cstr_at psize_a(_in cstr_at src);
	unsigned size_a(_in cstr_at src);

	str_at pcopy_a(_out str_at dst, _in unsigned dst_size, _in cstr_at src);			// return dst-ptr to '\0'
	template <unsigned dst_size> str_at pcopy_a(
		_out char_at dst[dst_size], _in cstr_at src
	) {
		return pcopy_a(dst, dst_size, src);
	}
	unsigned copy_a(_out str_at dst, _in unsigned dst_size, _in cstr_at src);			// return copied chars count
	template <unsigned dst_size> unsigned copy_a(
		_out char_at dst[dst_size], _in cstr_at src
	) {
		return copy_a(dst, dst_size, src);
	}

	//template <case_sensitivity case_sensitivity> struct compare {
	//	static char function(_in cstr_at lhs, _in cstr_t rhs);
	//	static char function(_in const string_at &lhs, _in const string_at &rhs);

	//	struct equal {
	//	};
	//	struct less {
	//	};
	//};
}

namespace com {
	class port {

	public:
		typedef byte_t number;
		typedef void *handle;

		struct config {

			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-dcb
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

			// https://docs.microsoft.com/ru-ru/windows/win32/api/winbase/ns-winbase-commtimeouts
			struct timeouts_t {
				struct total_t {
					unsigned multiplier, constant;
				};
				struct read_t {
					unsigned interval;
					total_t total;
				};
				typedef struct total_t write_t;

				read_t read;
				write_t write;
			};
			timeouts_t timeouts;

			unsigned buffer_size;

			config(_in const dcb_t &dcb, _in timeouts_t timeouts, _in unsigned buffer_size = 4096);
		};

	public:
		static std::vector<number> static__enum();

		port();
		port(_in const port &port) = delete;
		port(_in _out port &&port);
		~port();
		
		port& operator =(_in const port &port) = delete;
		port& operator =(_in _out port &&port);
		//bool operator ==(_in const port &port) const;
		//bool operator !=(_in const port &port) const;
		
		set_lasterror(bool) open(_in number number, _in const config &config);
		set_lasterror(bool) close();

		set_lasterror(unsigned) send(_in cstr_at str) const;
		set_lasterror(unsigned) send(_in const string_at &string) const;
		set_lasterror(cstr_at) recieve(_out string_at &string) const;
		set_lasterror(string_at) recieve() const;

	protected:
		struct buffer {
			typedef char char_t;

			std::unique_ptr<char_t> data;
			unsigned size = 0;

			buffer();
			buffer(_in const buffer &buffer) = delete;
			buffer(_in _out buffer &&buffer);

			buffer& operator =(_in const buffer &buffer) = delete;
			buffer& operator =(_in _out buffer &&buffer);

			bool is_empty() const;
		};

	protected:
		// bytes sended (not more then buffer.size)
		static set_lasterror(unsigned) static__send(_in handle handle, _in const buffer &buffer, _in unsigned count);
		// bytes recieved (not more then buffer.size)
		static set_lasterror(unsigned) static__recieve(_in handle handle, _out buffer &buffer);
		// not nullptr on success, nullptr on error
		static set_lasterror(handle) static__open(_in number number, _in const config::dcb_t &config_dcb, _in const config::timeouts_t &timeouts);
		// nullptr on success, same handle on error
		static set_lasterror(handle) static__close(_in handle handle);

		//static bool static__compare(_in const port &lhs, _in const port &rhs);
		//bool compare(_in const port &port) const;		

	private:
		handle _handle;
		mutable buffer _buffer;
	};

	struct at {
		template <typename type> struct result {
			type data, status;
		};
	};
}
