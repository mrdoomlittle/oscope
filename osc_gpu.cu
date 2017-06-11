# include "config.h"
# include <eint_t.hpp>
# include <cstdio>

__global__ void gpu_shift_left(mdl::uint_t *__xa_len, mdl::u8_t *__pm_frame, mdl::u8_t *__pmf_buff) {
	mdl::uint_t point = (threadIdx.x + (blockIdx.x * *__xa_len)) * 3;

	__pmf_buff[point] = __pm_frame[point + 3];
	__pmf_buff[point + 1] = __pm_frame[point + 1 + 3];
	__pmf_buff[point + 2] = __pm_frame[point + 2 + 3];
}

__global__ void gpu_draw_ib_buff(mdl::uint_t *__xa_len, mdl::u8_t *__pm_frame, mdl::u8_t *__ib_buff) {
	mdl::uint_t point = ((threadIdx.x + ((*__xa_len) - IB_BUFF_SIZE)) + (blockIdx.x * *__xa_len)) * 3;
	if (blockIdx.x < __ib_buff[threadIdx.x]) {
		__pm_frame[point] = 222;
		__pm_frame[point + 1] = 222;
		__pm_frame[point + 2] = 222;
	} else {
		__pm_frame[point] = 0;
		__pm_frame[point + 1] = 0;
		__pm_frame[point + 2] = 0;
	}
}

void build_frame(mdl::uint_t __xa_len, mdl::uint_t __ya_len, mdl::u8_t *__pm_frame, mdl::u8_t *__ib_buff) {
	bool static inited = false;
	mdl::uint_t static *xa_len;

	mdl::u8_t static *pm_frame = nullptr;
	mdl::u8_t static *pmf_buff = nullptr;
	mdl::u8_t static *ib_buff = nullptr;
	cudaError_t any_err = cudaSuccess;

	if (!inited) {
		if ((any_err = cudaMalloc((void **)&xa_len, sizeof(mdl::uint_t))) != cudaSuccess) {
			fprintf(stderr, "failed to alloc memory, errno: %d\n", any_err);
			return;
		}

		if ((any_err = cudaMalloc((void **)&ib_buff, IB_BUFF_SIZE*sizeof(mdl::u8_t))) != cudaSuccess) {
			fprintf(stderr, "failed to alloc memory, errno: %d\n", any_err);
			return;
		}

		inited = true;
	}

	mdl::uint_t static _xa_len{};
	mdl::uint_t static _ya_len{};

	if (_xa_len != __xa_len || _ya_len != __ya_len) {
		if (pm_frame != nullptr) cudaFree(pm_frame);
		if (pmf_buff != nullptr) cudaFree(pmf_buff);

		cudaMalloc((void **)&pm_frame, (__xa_len * __ya_len) * 3);
		cudaMalloc((void **)&pmf_buff, (__xa_len * __ya_len) * 3);

		if (_xa_len != __xa_len) {
			if ((any_err = cudaMemcpy(xa_len, &__xa_len, sizeof(mdl::uint_t), cudaMemcpyHostToDevice)) != cudaSuccess) {
				fprintf(stderr, "failed to copy memory to device, errno: %d\n", any_err);
				return;
			}
		}

		cudaMemset(pmf_buff, 0, (__xa_len * __ya_len) * 3);

		_xa_len = __xa_len;
		_ya_len = __ya_len;
	}

	cudaMemcpy(pm_frame, __pm_frame, (__xa_len * __ya_len) * 3, cudaMemcpyHostToDevice);
	cudaMemcpy(ib_buff, __ib_buff, IB_BUFF_SIZE*sizeof(mdl::u8_t), cudaMemcpyHostToDevice);

	gpu_draw_ib_buff<<<__ya_len, IB_BUFF_SIZE>>>(xa_len, pm_frame, ib_buff);

	for (mdl::uint_t sc{}; sc != IB_BUFF_SIZE; sc ++) {
		gpu_shift_left<<<__ya_len, __xa_len - 1>>>(xa_len, pm_frame, pmf_buff);
		cudaMemcpy(pm_frame, pmf_buff, (__xa_len * __ya_len) * 3, cudaMemcpyDeviceToDevice);
	}

	cudaMemcpy(__pm_frame, pmf_buff, (__xa_len * __ya_len) * 3, cudaMemcpyDeviceToHost);
}
