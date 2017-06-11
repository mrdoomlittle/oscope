# include <boost/asio/serial_port.hpp>
# include <boost/asio.hpp>
# include <cstdio>
# include <eint_t.hpp>
# include <chrono>
# include <math.h>
# include <sys/ioctl.h>
# include <termios.h>
int main() {
	boost::asio::io_service io_service;
	boost::asio::serial_port serial_port(io_service, "/dev/ttyACM0");
	serial_port.set_option(boost::asio::serial_port_base::baud_rate(19200));

	do {
		try {
			mdl::u8_t incomming_byte = 0;
		//	::tcflush(serial_port.lowest_layer().native_handle(), TCIFLUSH);
			boost::asio::read(serial_port, boost::asio::buffer(&incomming_byte, sizeof(mdl::u8_t)));
			printf("%d\n", incomming_byte);
		} catch(boost::system::system_error const& e) {}
	} while(1);
}
