all: clean
	nvcc -std=c++11 -c -o osc_gpu.o osc_gpu.cu
	g++ -std=c++11 -I/usr/local/cuda-8.0/include -L/usr/local/cuda-8.0/lib64 -o oscope oscope.cpp osc_gpu.o -lcuda -lcudart -lGL -lGLU -lglut -lboost_system -lboost_thread -lpthread
clean:
	rm -f *.o oscope
