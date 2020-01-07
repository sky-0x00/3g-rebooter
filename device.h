#pragma once

#include "utils.h"

class device {
public:
	struct find_info {

		struct com_t {
			string_at vendor, model;
		};
		com_t com;
		struct usb_t {
			string_t driver_desc;
		} usb;

		typedef std::vector<::com::port::number> ports_t;
		ports_t ports;

		find_info(_in const com_t &com, _in const usb_t &usb, _in const ports_t &ports = {});
	};
	com::port::number find(_in const find_info &find_info);											// 0 on not-found

	class sms {
	public:
		class info {
		public:
			// sms-type						// AT+CSMS=<V>, AT+CSMS?, AT+CSMS=?
			// sms-format					// AT+CMGF=<V>, AT+CMGF?, AT+CMGF=?
			enum class format {
				pdu = 0, text = 1
			};
			//bool get_format(_out format &format);
			format get_format() const;
		public:
			info(_in const device &device);
		private:
			const device &_device;
		};		

		struct message {
			typedef std::vector<byte_t> pdu_t;

			enum class state_t: unsigned {
				state_0 = 0,				// Unread message that has been received
				state_1,					// Read message that has been received
				state_2,					// Unsent message that has been stored
				state_3						// Sent message that has been stored
			};			

			state_t state;
			unsigned size_tpdu;				// ?
			pdu_t pdu;					// pdu octets
		};

		bool read_message(_in unsigned index, _out message &message) const;
		message::state_t read_message(_in unsigned index, _out message::pdu_t &pdu) const;

	public:
		sms(_in const device &device);
	private:
		const device &_device;
	};

	cstr_at at(_in cstr_at in, _out string_at &out) const;
	string_at at(_in cstr_at in) const;
	cstr_at check_for_data(_out string_at &data) const;
	string_at check_for_data() const;

protected:
	static bool static__check_com(_in const find_info::com_t &com, _in const com::port &cp);
	static bool static__check_usb(_in const find_info::usb_t &usb, _in com::port::number cpn);
	static com::port::number static__find(_in const find_info &find_info, _out com::port &cp);		// 0 on not-found
	
	static cstr_at static__at(_in const com::port &cp, _in cstr_at in, _out string_at &out);
	static string_at static__at(_in const com::port &cp, _in cstr_at in);

	static cstr_at static__com__check_for_data(_in const com::port &cp, _out string_at &data);
	static string_at static__com__check_for_data(_in const com::port &cp);

private:
	static void static__buffer_clear(_in const com::port &cp);
private:
	com::port _cp;
};