# include <eint_t.hpp>
# include <boost/asio/serial_port.hpp>
# include <boost/asio.hpp>
# include <boost/thread.hpp>
# include <GL/glut.h>
# include <cstdlib>
# include <string.h>
# include <cuda_runtime.h>
# include <GL/freeglut.h>
# include <malloc.h>
# include "config.h"
# include <atomic>
mdl::u8_t inbound_buff[IB_BUFF_SIZE];
mdl::uint_t ib_buff_point = 0;

mdl::u8_t *frame_pm = nullptr;
boost::asio::io_service io_service;
boost::asio::serial_port serial_port(io_service, "/dev/ttyACM0");

bool osc_to_shutdown = false;
std::atomic<bool> unload_ib_buff;

extern void build_frame(mdl::uint_t, mdl::uint_t, mdl::u8_t*, mdl::u8_t*);

void osc_tick() {
	if (unload_ib_buff) {
		build_frame(WD_XA_LEN, WD_YA_LEN, frame_pm, inbound_buff);
		printf("unloading inbound buffer.\n");
		glDrawPixels(WD_XA_LEN, WD_YA_LEN, GL_RGB, GL_UNSIGNED_BYTE, frame_pm);
		ib_buff_point = 0;
		unload_ib_buff = false;
	}

	glutSwapBuffers();
	glutPostRedisplay();
}

void osc_recv_serial() {
	do {
		try {
			if (ib_buff_point == IB_BUFF_SIZE - 1) {
				unload_ib_buff = true;
				while(unload_ib_buff){}
			}
			::tcflush(serial_port.lowest_layer().native_handle(), TCIFLUSH);
			boost::asio::read(serial_port, boost::asio::buffer(inbound_buff + ib_buff_point, 1));

			ib_buff_point++;
		} catch(boost::system::system_error const& e) {continue;}
	} while(!osc_to_shutdown);
}

int main(int argc, char *argv[]) {
	unload_ib_buff = false;
	frame_pm = (mdl::u8_t*)malloc(WD_XA_LEN*WD_YA_LEN*3);
	bzero(frame_pm, WD_XA_LEN*WD_YA_LEN*3);

	serial_port.set_option(boost::asio::serial_port_base::baud_rate(OSC_BAUDRATE));
	boost::thread th{osc_recv_serial};

	glutInit(&argc, argv);
	glutInitWindowSize(WD_XA_LEN, WD_YA_LEN);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
	glutCreateWindow("Oscope");
	glutDisplayFunc(osc_tick);
	glutMainLoop();

	cudaDeviceReset();
	std::free(frame_pm);
}
