#pragma once
#include <memory>

namespace bitness {

	enum /*class*/ value: bool {
		x32, x64
	};

	class interface abstract {
	public:
		value get();
		virtual ~interface() = default;
	protected:
		virtual value __get() const = 0;				// static value static__get();
	private:
		std::unique_ptr<value> _value;
	};

	class process final: public interface {
	public:
		static bool static__is_wow64();
		bool is_wow64();
		static constexpr value static__get() noexcept;
	protected:
		virtual value __get() const override;
	private:
		std::unique_ptr<bool> _wow64;
	};

	class system final: public interface {
	public:
		static value static__get();
	protected:
		virtual value __get() const override;
	};
}
