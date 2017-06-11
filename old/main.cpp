
# include <cstdio>
# include <boost/asio/serial_port.hpp>
# include <boost/asio.hpp>
# include <eint_t.hpp>
# include <GL/glut.h>
# include <cstdlib>
# include <string.h>
# include <cuda_runtime.h>
# include <boost/thread.hpp>
# include <GL/freeglut.h>
# include <mutex>
std::mutex mtx;
boost::asio::io_service io_service;
boost::asio::serial_port serial_port(io_service, "/dev/ttyACM0");
mdl::u8_t incomming_byte = 0x0;
# define WD_XA_LEN 900
# define WD_YA_LEN 300
mdl::u8_t *pm = NULL;
void extern shift_left(mdl::uint_t, mdl::uint_t, mdl::u8_t*, mdl::uint_t);
void extern cu_init();
void extern cu_de_init();

# define BUFF_SIZE 10

# define LIN_WIDTH 1
mdl::u8_t r = 255, g = 0, b = 0;


mdl::u8_t inbound_byte = 0;
mdl::uint_t ib_buff_point = 0;
mdl::u8_t inbound_buff[BUFF_SIZE];

void render() {
	mdl::u8_t byte_buff = 0x0;
	byte_buff = incomming_byte;

	for (mdl::uint_t ya{}; ya != WD_YA_LEN; ya ++) {
		for (mdl::uint_t xa{WD_XA_LEN - LIN_WIDTH}; xa != WD_XA_LEN; xa ++) {
			mdl::uint_t point = (xa + (ya * WD_XA_LEN)) * 3;
			if (ya < byte_buff) {
				pm[point] = r;
				pm[point + 1] = g;
 				pm[point + 2] = b;
			} else {
				pm[point] = pm[point + 1] = pm[point + 2] = 0x0;
			}
		}
	}

	shift_left(WD_XA_LEN, WD_YA_LEN, pm, LIN_WIDTH);
}
bool run = true;
bool wait = false, is_waiting = false;
void get_serial() {
	do {
//		::tcflush(serial_port.lowest_layer().native_handle(), TCIFLUSH);
		try {
			boost::asio::read(serial_port, boost::asio::buffer(&inbound_byte, 1));
			incomming_byte = byte_buff;

//			render();
//			while(wait) {if (!is_waiting) is_waiting = true;}
//			is_waiting = false;

		} catch(boost::system::system_error const& e) {continue;}
	} while(run);
	run = true;
}
# include <chrono>
static std::chrono::high_resolution_clock::time_point begin_point = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point now_point;
void tick() {
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	render();
	now_point = std::chrono::high_resolution_clock::now();
	if (std::chrono::duration_cast<std::chrono::milliseconds>(now_point - begin_point).count() <= 1000/80) {glutPostRedisplay();return;}

	wait = true;
	while(!is_waiting) {}
	glDrawPixels(WD_XA_LEN, WD_YA_LEN, GL_RGB, GL_UNSIGNED_BYTE, pm);
	wait = false;
	glutSwapBuffers();
	//printf("tick. byte: %d\n", incomming_byte);
	glutPostRedisplay();
	begin_point = std::chrono::high_resolution_clock::now();
}

int main(int argc, char *argv[]) {
	pm = (mdl::u8_t *)malloc(WD_XA_LEN * WD_YA_LEN * 3);
	bzero(pm, WD_XA_LEN * WD_YA_LEN * 3);
	cu_init();
	serial_port.set_option(boost::asio::serial_port_base::baud_rate(19200));
	boost::thread th{get_serial};
	glutInit(&argc, argv);

	glutInitWindowSize(WD_XA_LEN, WD_YA_LEN);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
	glutCreateWindow("Oscope");
	glutDisplayFunc(tick);
	glutMainLoop();
	run = false;
	while(!run){}
	cu_de_init();
	std::free(pm);
	cudaDeviceReset();
	printf("deinit.\n");
}
