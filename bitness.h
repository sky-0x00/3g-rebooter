#pragma once
#include <memory>

namespace bitness {

	enum /*class*/ value: bool {
		x32, x64
	};

	class interface abstract {
	public:
		static constexpr value static__get() /*= 0*/;
		virtual value get() final;
		virtual ~interface() = default;
	protected:
		std::unique_ptr<value> _value;
	};

	class process: public interface {
	public:
		static constexpr value static__get() noexcept;
		static bool static__is_wow64();
		bool is_wow64();
	protected:
		std::unique_ptr<bool> _is_wow64;
	};
	class system: public interface {
	public:
		static value static__get();
	};
}
