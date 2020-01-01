#include "pch.h"
#include "application.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------
application::application(
	_in const config &config /*= {}*/
) :
	_config(config)
{}

//------------------------------------------------------------------------------------------------------------------------------------------------------
application::config::config(
	_in bool is_service, _in const poling_t &poling /*= {10}*/
) :
	is_service(is_service), poling(poling)
{}
application::config::config(
	_in const config &config
) :
	is_service(config.is_service), poling(config.poling)
{}

/*explicit*/ application::config::config(
	_in argc_t argc, _in const argv_t &argv
) :
	config(static__get(argc, argv))
{}

bool /*static*/ application::config::static__get(
	_in argc_t /*argc*/, _in const argv_t &/*argv*/, _out config &config
) noexcept {
	config = { false, {10} };		// temporary, need parser's implementation
	return true;
}
application::config /*static*/ application::config::static__get(
	_in argc_t argc, _in const argv_t &argv
) {
	config config {false};
	if (static__get(argc, argv, config))
		return config;
	trace(L"static__get(): false");
	throw -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------