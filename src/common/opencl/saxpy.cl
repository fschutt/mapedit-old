R"(
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

//	typedef char				int8_t;
//	typedef unsigned char		uint8_t;
//
//	typedef short				int16_t;
//	typedef unsigned short		uint16_t;
//
//	typedef int					int32_t;
//	typedef unsigned int		uint32_t;
//
//	typedef long				int64_t;
//	typedef unsigned long 		uint64_t;
//
//	Restricted to device:
//		bool size_t ptrdiff_t intptr_t uintptr_t
//
//	Memory regions:
//		__global __local __private __constant
//
//	Vector types:
//		vec.s0 ... vec.s8 .... vec.sA .... vec.sF
//
//	How to declare vectors:
//		v2, v4, v8, v16
//
//	structs unions
//
//	get_global_id(dimension)
//	get_global_offset(dimension)
//
//	get_local_id(dimension)
//
//	vec_type_hint
//	reqd_work_group_size
//
//	Enable double precision:
//	#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void saxpy_kernel(__global float* x, __global float* y, float a)
{
	const int i = get_global_id (0);
	y [i] += a * x [i];
}

)"
