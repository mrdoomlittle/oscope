# include <eint_t.hpp>
# include <cstdio>
__global__ void cu_shift_left(mdl::uint_t *__xa_len, mdl::u8_t *__0pm, mdl::u8_t *__1pm) {
	mdl::uint_t point = (threadIdx.x + (blockIdx.x * *__xa_len)) * 3;
	__0pm[point] = __1pm[point + 3];
	__0pm[point + 1] = __1pm[point + 1 + 3];
	__0pm[point + 2] = __1pm[point + 2 + 3];
}

void shift_left(mdl::uint_t __xa_len, mdl::uint_t __ya_len, mdl::u8_t *__pm, mdl::uint_t __shift_amount) {
	bool static inited = false;
	mdl::u8_t static* _0pm = nullptr;
	mdl::u8_t static *_1pm = nullptr;
	mdl::uint_t static*xa_len;
	cudaError_t any_err = cudaSuccess;
	if (!inited) {
		if ((any_err = cudaMalloc((void **)&xa_len, sizeof(mdl::uint_t))) != cudaSuccess) {
			fprintf(stderr, "failed to alloc memory, errno: %d\n", any_err);
		}

		inited = true;
	}

	mdl::uint_t static _xa_len{};
	mdl::uint_t static _ya_len{};
	if (_xa_len != __xa_len || _ya_len != __ya_len) {
		if (_0pm != nullptr) cudaFree(_0pm);
		if (_1pm != nullptr) cudaFree(_1pm);

		cudaMalloc((void **)&_0pm, (__xa_len * __ya_len) * 3);
		cudaMalloc((void **)&_1pm, (__xa_len * __ya_len) * 3);
		if (_xa_len != __xa_len) {
			if ((any_err = cudaMemcpy(xa_len, &__xa_len, sizeof(mdl::uint_t), cudaMemcpyHostToDevice)) != cudaSuccess) {
				fprintf(stderr, "failed to copy memory to device, errno: %d\n", any_err);
			}
		}

		_xa_len = __xa_len;
		_ya_len = __ya_len;
	}

	cudaMemcpy(_0pm, __pm, (__xa_len * __ya_len) * 3, cudaMemcpyHostToDevice);
	for (mdl::uint_t sa{}; sa != __shift_amount; sa ++) {
		cudaMemcpy(_1pm, _0pm, (__xa_len * __ya_len) * 3, cudaMemcpyDeviceToDevice);
		cu_shift_left<<<__ya_len, __xa_len - 1>>>(xa_len, _0pm, _1pm);
	}


	cudaMemcpy(__pm, _0pm, (__xa_len * __ya_len) * 3, cudaMemcpyDeviceToHost);
}

void cu_init() {

}

void cu_de_init() {

}


