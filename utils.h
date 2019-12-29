#pragma once

#include "pch.h"
#include <vector>

namespace com {
	class port {
	public:
		typedef byte_t number;
		typedef void *handle;
		struct config {

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
	public:
		static std::vector<number> static__enum();
		port();
		~port();
		set_lasterror(bool) open(_in number number, _in const config &config);
		set_lasterror(bool) close();
	protected:
		static set_lasterror(handle) static__open(_in number number, _in const config &config);
		static set_lasterror(bool) static__close(_in handle handle);
	private:
		handle _handle;
	};
}