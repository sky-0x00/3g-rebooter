#pragma once

#include "pch.h"
#include <vector>
#include <cassert>
#include <utility>
#include <initializer_list>

namespace stdex {
	template <typename type> bool is_any(_in type value, _in std::initializer_list<type> range) {
		return range.end() != std::find(range.begin(), range.end(), value);
	}

	template <typename type> bool is__in_range(_in type value, _in const std::pair<type, type> range) {
		assert(range.second >= range.first);
		return (value >= range.first) && (value < range.second);
	}
	template <typename type> bool is__in_range__inclusive(_in type value, _in const std::pair<type, type> range) {
		assert(range.second >= range.first);
		return (value >= range.first) && (value <= range.second);
	}
}

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

	template <typename type> struct view_at;
	template <> struct view_at<string_at::const_iterator> : std::pair<string_at::const_iterator, string_at::const_iterator> {
		view_at(
			_in string_at::const_iterator first, _in string_at::const_iterator second
		) : 
			std::pair<string_at::const_iterator, string_at::const_iterator>(first, second)
		{}
		unsigned size(
		) const {
			const auto &distance = std::distance(first, second);
			assert(0 <= distance);
			return static_cast<unsigned>(distance);
		}
		string_at string(
		) const {
			return {first, second};
		}
		operator string_at(
		) const {
			return string();
		}
	};
	template <> struct view_at<str_at> : std::pair<cstr_at, cstr_at> {
		view_at(
			_in cstr_at first, _in cstr_at second
		) :
			std::pair<cstr_at, cstr_at>(first, second)
		{}
		unsigned size(
		) const {
			const auto &distance = std::distance(first, second);
			assert(0 <= distance);
			return static_cast<unsigned>(distance);
		}
		string_at string(
		) const {
			return { first, static_cast<string_at::size_type>(std::distance(first, second)) };
		}
		operator string_at(
		) const {
			return string();
		}
	};

	const string_at& to_upper(_in _out string_at &string);
	const string_at& to_lower(_in _out string_at &string);
	const string::view_at<string_at::const_iterator>& to_upper(_in _out string::view_at<string_at::const_iterator> &string_view);
	const string::view_at<string_at::const_iterator>& to_lower(_in _out string::view_at<string_at::const_iterator> &string_view);

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
		typedef /*byte_t*/ unsigned number;
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
		typedef std::vector<string::view_at<string_at::const_iterator>> match;
		struct result {

			constexpr static inline cstr_at ok() noexcept {
				return "OK";
			}
			
			result() = default;
			//result(_in const result &) = delete;
			//result(_in _out result &&result);
			
			//result& operator =(_in const result &) = delete;
			//result& operator =(_in _out result &&result);
			
			void clear();

			string_at data;
			match match;
		};

		class check {
		public:
			check(_in const string_at &string);
			
			bool find(_out match &match) const;		// or common
			bool new_sms(_out match &match) const;
			bool sms_read(_out match &match) const;
			bool sms_format(_out match &match) const;

		protected:
			bool process(_in cstr_at regex_pattern, _out match &match) const noexcept;
			//match process(_in cstr_at regex_pattern) const;

		private:
			const string_at &_string;
		};
	};

}	// namespace com

namespace pdu {
	typedef std::vector<byte_t> encoded;			// same as "::device::sms::message::pdu_t"
	typedef string_at number;
	struct decoded {
		number smsc;
		struct /*message*/ {
			byte_t id;
			struct /*content*/ {
				number sender;
				string_at text;
			} content;
		} message;
	};
	bool decode(_in const encoded &encoded, _in unsigned size_tpdu, _out decoded &decoded);

	// todo: class decoder;
};

namespace console {
	class cursor_position {
	public:
		struct value {
			short x, y;
		};
		
		static value get();
		static void set(_in const value &value);

	protected:
		static bool safe__get(_out value &value) noexcept;
		static bool safe__set(_in const value &value) noexcept;
	};
}

constexpr unsigned WaitObject(_in unsigned Id) noexcept {
	return /*WAIT_OBJECT_0*/0 + Id;
}
