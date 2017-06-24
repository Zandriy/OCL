#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY

#include <CL/cl2.hpp>

#include <iostream>
#include <string>
#include <exception>
#include <cstdlib>
#include <ctime>

// CL program source
const std::string kernel1{ R"KS1(
#if defined(cl_khr_fp64)
#  pragma OPENCL EXTENSION cl_khr_fp64: enable
#elif defined(cl_amd_fp64)
#  pragma OPENCL EXTENSION cl_amd_fp64: enable
#else
#  error double precision is not supported
#endif
kernel
void entry_point(ulong n, global const double *a,
        global const double *b, global double *c)
{
    size_t id = get_global_id(0);
    if (id < n)
       c[id] =  pow(a[id], b[id]);
}
)KS1" };

const size_t N = 0xFFFFFF;

enum CLver : uint8_t {
	v100,
	v110,
	v120,
	v200,
	v210,
	v220,
	vUnknown
};

CLver CLver_num[] {v100, v110, v120, v200, v210, v220};
const char* CLver_str[] {"OpenCL 1.0", "OpenCL 1.1", "OpenCL 1.2", "OpenCL 2.0", "OpenCL 2.1", "OpenCL 2.2"};

CLver getCL_ver(const std::string& ver)
{
	CLver cur_ver{ vUnknown };
	for (CLver v : CLver_num)
	{
		if (ver.find(CLver_str[v]) == 0)
		{
			cur_ver = v;
		}
	}
	return cur_ver;
}
    
void printCL_PlatformInfo(const cl::Platform& platform)
{
    std::string info;
    std::cout << "\n=====================================\n";
    std::cout << "========== PLATFORM INFO ============\n";
    if (platform.getInfo(CL_PLATFORM_PROFILE, &info) == CL_SUCCESS)
        std::cout << "PROFILE: " << info << "\n";
    if (platform.getInfo(CL_PLATFORM_VERSION, &info) == CL_SUCCESS)
        std::cout << "VERSION: " << info << "\n";
    if (platform.getInfo(CL_PLATFORM_NAME, &info) == CL_SUCCESS)
        std::cout << "NAME: " << info << "\n";
    if (platform.getInfo(CL_PLATFORM_VENDOR, &info) == CL_SUCCESS)
        std::cout << "VENDOR: " << info << "\n";
    if (platform.getInfo(CL_PLATFORM_EXTENSIONS, &info) == CL_SUCCESS)
        std::cout << "EXTENSIONS: " << info << "\n";
    std::cout << "=====================================\n";
}

cl::Platform getCL_Platform()
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	size_t cur_platform = platforms.size();

	if (!cur_platform)
	{
		throw std::domain_error("OpenCL platforms aren't found.");
	}

	for (size_t p = 0; p < platforms.size(); ++p)
    {
        printCL_PlatformInfo(platforms.at(p));
        std::string ver;
        if (platforms.at(p).getInfo(CL_PLATFORM_VERSION, &ver) == CL_SUCCESS)
        {
            if (getCL_ver(ver) == v120 || (getCL_ver(ver) >= v120 && cur_platform == platforms.size()))
            {
                cur_platform = p;
            }
        }
	}

	if (cur_platform == platforms.size())
	{
		throw std::domain_error("OpenCL 1.2 platform is not found.");
	}

	return std::move(platforms.at(cur_platform));
}
    
void printCL_DeviceInfo(const cl::Device& dev)
{
    std::cout << "\n+++++++++++++++++++++++++++++++++++++";
    std::cout << "\n+++++++++++ DEVICE INFO +++++++++++++";
    std::cout << "\nTYPE: ";
    switch(dev.getInfo<CL_DEVICE_TYPE>())
    {
        case CL_DEVICE_TYPE_CPU:
            std::cout << "CPU";
            break;
        case CL_DEVICE_TYPE_GPU:
            std::cout << "GPU";
            break;
        case CL_DEVICE_TYPE_ACCELERATOR:
            std::cout << "ACCELERATOR";
            break;
        case CL_DEVICE_TYPE_DEFAULT:
            std::cout << "DEFAULT";
            break;
    }
    std::cout << " | VENDOR_ID: " << dev.getInfo<CL_DEVICE_VENDOR_ID>();
    std::cout << " | NAME: " << dev.getInfo<CL_DEVICE_NAME>();
    std::cout << " | VENDOR: " << dev.getInfo<CL_DEVICE_VENDOR>();
    std::cout << " | DRIVER_VERSION: " << dev.getInfo<CL_DRIVER_VERSION>();
    std::cout << " | PROFILE: " << dev.getInfo<CL_DEVICE_PROFILE>();
    std::cout << " | VERSION: " << dev.getInfo<CL_DEVICE_VERSION>();
    std::cout << " | PLATFORM: " << dev.getInfo<CL_DEVICE_PLATFORM>();
    std::cout << " | AVAILABLE: " << (dev.getInfo<CL_DEVICE_AVAILABLE>() ? "YES" : "NO");
    std::cout << " | COMPILER_AVAILABLE: " << (dev.getInfo<CL_DEVICE_COMPILER_AVAILABLE>() ? "YES" : "NO");
    std::cout << " | OPENCL_C_VERSION: " << dev.getInfo<CL_DEVICE_OPENCL_C_VERSION>();
    std::cout << " | PARENT_DEVICE: ";
    auto parent = dev.getInfo<CL_DEVICE_PARENT_DEVICE>();
    auto g = parent.get();
 //   std::cout << (parent.get() ? parent.getInfo<CL_DEVICE_NAME>() : "NULL");
    std::cout << " | ADDRESS_BITS: " << dev.getInfo<CL_DEVICE_ADDRESS_BITS>();
    std::cout << " | MEM_BASE_ADDR_ALIGN: " << dev.getInfo<CL_DEVICE_MEM_BASE_ADDR_ALIGN>();
    std::cout << " | MIN_DATA_TYPE_ALIGN_SIZE: " << dev.getInfo<CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE>();
    std::cout << " | CONSTANT_BUFFER_SIZE: " << dev.getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>();
    std::cout << " | ERROR_CORRECTION_SUPPORT: " << (dev.getInfo<CL_DEVICE_ERROR_CORRECTION_SUPPORT>() ? "YES" : "NO");
    std::cout << " | PROFILING_TIMER_RESOLUTION: " << dev.getInfo<CL_DEVICE_PROFILING_TIMER_RESOLUTION>();
    std::cout << " | ENDIAN_LITTLE: " << (dev.getInfo<CL_DEVICE_ENDIAN_LITTLE>() ? "YES" : "NO");
    std::cout << " | EXECUTION_CAPABILITIES: ";
    switch(dev.getInfo<CL_DEVICE_EXECUTION_CAPABILITIES>())
    {
        case CL_EXEC_KERNEL:
            std::cout << "KERNEL";
            break;
        case CL_EXEC_NATIVE_KERNEL:
            std::cout << "NATIVE_KERNEL";
            break;
            
    }
    std::cout << " | QUEUE_PROPERTIES: ";
    switch(dev.getInfo<CL_DEVICE_QUEUE_PROPERTIES>())
    {
        case CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
            std::cout << "OUT_OF_ORDER_EXEC_MODE_ENABLE";
            break;
        case CL_QUEUE_PROFILING_ENABLE:
            std::cout << "PROFILING_ENABLE";
            break;
    }
    std::cout << " | HOST_UNIFIED_MEMORY: " << (dev.getInfo<CL_DEVICE_HOST_UNIFIED_MEMORY>() ? "YES" : "NO");
    std::cout << " | BUILT_IN_KERNELS: " << dev.getInfo<CL_DEVICE_BUILT_IN_KERNELS>();
    std::cout << " | REFERENCE_COUNT: " << dev.getInfo<CL_DEVICE_REFERENCE_COUNT>();
    //std::cout << " | LINKER_AVAILABLE: " << (dev.getInfo<CL_DEVICE_LINKER_AVAILABLE>() ? "YES" : "NO");
    //std::cout << " | PRINTF_BUFFER_SIZE: " << dev.getInfo<CL_DEVICE_PRINTF_BUFFER_SIZE>();
    
    std::cout << "\n= EXTENSIONS =\n" << dev.getInfo<CL_DEVICE_EXTENSIONS>();
    
    std::cout << "\n= NATIVE_VECTOR_WIDTH =\nCHAR: " << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR>();
    std::cout << " | SHORT: " << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT>();
    std::cout << " | INT: " << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_INT>();
    std::cout << " | LONG: " << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG>();
    std::cout << " | FLOAT: " << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT>();
    std::cout << " | DOUBLE: " << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE>();
    std::cout << " | HALF: " << dev.getInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF>();
    
    std::cout << "\n= REFERRED_VECTOR_WIDTH =\nCHAR: " << dev.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR>();
    std::cout << " | SHORT: " << dev.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT>();
    std::cout << " | INT: " << dev.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT>();
    std::cout << " | LONG: " << dev.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG>();
    std::cout << " | FLOAT: " << dev.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>();
    std::cout << " | DOUBLE: " << dev.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>();
    std::cout << " | HALF: " << dev.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF>();
    
    std::cout << "\nPREFERRED_INTEROP_USER_SYNC: " << dev.getInfo<CL_DEVICE_PREFERRED_INTEROP_USER_SYNC>();
    
    std::cout << "\n= MAX_WORK =\nITEM_DIMENSIONS: " << dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>();
    std::cout << " | GROUP_SIZE: " << dev.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
    std::cout << " | ITEM_SIZES: ";
    auto sizes = dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
    for(auto s : sizes)
    {
        std::cout << s << ", ";
    }
    
    std::cout << "\n= MAX =\nCOMPUTE_UNITS: " << dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
    std::cout << " | CLOCK_FREQUENCY: " << dev.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
    std::cout << " | READ_IMAGE_ARGS: " << dev.getInfo<CL_DEVICE_MAX_READ_IMAGE_ARGS>();
    std::cout << " | WRITE_IMAGE_ARGS: " << dev.getInfo<CL_DEVICE_MAX_WRITE_IMAGE_ARGS>();
    std::cout << " | MEM_ALLOC_SIZE: " << dev.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
    std::cout << " | PARAMETER_SIZE: " << dev.getInfo<CL_DEVICE_MAX_PARAMETER_SIZE>();
    std::cout << " | SAMPLERS: " << dev.getInfo<CL_DEVICE_MAX_SAMPLERS>();
    std::cout << " | CONSTANT_ARGS: " << dev.getInfo<CL_DEVICE_MAX_CONSTANT_ARGS>();
    
    
    std::cout << "\n= IMAGE =\nSUPPORT: " << (dev.getInfo<CL_DEVICE_IMAGE_SUPPORT>() ? "YES" : "NO");
    //std::cout << " | MAX_BUFFER_SIZE: " << dev.getInfo<CL_DEVICE_IMAGE_MAX_BUFFER_SIZE>();
    //std::cout << " | MAX_ARRAY_SIZE: " << dev.getInfo<CL_DEVICE_IMAGE_MAX_ARRAY_SIZE>();
    //std::cout << " | PITCH_ALIGNMENT: " << dev.getInfo<CL_DEVICE_IMAGE_PITCH_ALIGNMENT>();
    //std::cout << " | BASE_ADDRESS_ALIGNMENT: " << dev.getInfo<CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT>();
    std::cout << " | 2D_MAX_WIDTH: " << dev.getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>();
    std::cout << " | 2D_MAX_HEIGHT: " << dev.getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>();
    std::cout << " | 3D_MAX_WIDTH: " << dev.getInfo<CL_DEVICE_IMAGE3D_MAX_WIDTH>();
    std::cout << " | 3D_MAX_HEIGHT: " << dev.getInfo<CL_DEVICE_IMAGE3D_MAX_HEIGHT>();
    std::cout << " | 3D_MAX_DEPTH: " << dev.getInfo<CL_DEVICE_IMAGE3D_MAX_DEPTH>();
    
    std::cout << "\n= LOCAL_MEM =\nTYPE: ";
    switch(dev.getInfo<CL_DEVICE_LOCAL_MEM_TYPE>())
    {
        case CL_LOCAL:
            std::cout << "LOCAL";
            break;
        case CL_GLOBAL:
            std::cout << "GLOBAL";
            break;
    }
    std::cout << " | SIZE: " << dev.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();
    
    std::cout << "\n= GLOBAL_MEM =\nCACHE_TYPE: ";
    switch(dev.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE>())
    {
        case CL_NONE:
            std::cout << "NONE";
            break;
        case CL_READ_ONLY_CACHE:
            std::cout << "ONLY_CACHE";
            break;
        case CL_READ_WRITE_CACHE:
            std::cout << "READ_WRITE_CACHE";
            break;
    }
    std::cout << " | CACHELINE_SIZE: " << dev.getInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>();
    std::cout << " | CACHE_SIZE: " << dev.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>();
    std::cout << " | SIZE: " << dev.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
    
    
    std::cout << "\n= FP_CONFIG =\nSINGLE: ";
    switch(dev.getInfo<CL_DEVICE_SINGLE_FP_CONFIG>())
    {
        case CL_FP_DENORM:
            std::cout << "DENORM";
            break;
        case CL_FP_INF_NAN:
            std::cout << "INF_NAN";
            break;
        case CL_FP_ROUND_TO_NEAREST:
            std::cout << "ROUND_TO_NEAREST";
            break;
        case CL_FP_ROUND_TO_ZERO:
            std::cout << "ROUND_TO_ZERO";
            break;
        case CL_FP_ROUND_TO_INF:
            std::cout << "ROUND_TO_INF";
            break;
        case CL_FP_FMA:
            std::cout << "FMA";
            break;
    }
    std::cout << " | DOUBLE: ";
    switch(dev.getInfo<CL_DEVICE_DOUBLE_FP_CONFIG>())
    {
        case CL_FP_DENORM:
            std::cout << "DENORM";
            break;
        case CL_FP_INF_NAN:
            std::cout << "INF_NAN";
            break;
        case CL_FP_ROUND_TO_NEAREST:
            std::cout << "ROUND_TO_NEAREST";
            break;
        case CL_FP_ROUND_TO_ZERO:
            std::cout << "ROUND_TO_ZERO";
            break;
        case CL_FP_ROUND_TO_INF:
            std::cout << "ROUND_TO_INF";
            break;
        case CL_FP_FMA:
            std::cout << "FMA";
            break;
    }
    
    std::cout << "\n= PARTITION =\nAFFINITY_DOMAIN: ";
    switch(dev.getInfo<CL_DEVICE_PARTITION_AFFINITY_DOMAIN>())
    {
        case CL_DEVICE_AFFINITY_DOMAIN_NUMA:
            std::cout << "NUMA";
            break;
        case CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE:
            std::cout << "L4_CACHE";
            break;
        case CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE:
            std::cout << "L3_CACHE";
            break;
        case CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE:
            std::cout << "L2_CACHE";
            break;
        case CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE:
            std::cout << "L1_CACHE";
            break;
        case CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE:
            std::cout << "NEXT_PARTITIONABLE";
            break;
    }
    //std::cout << " | MAX_SUB_DEVICES: " << dev.getInfo<CL_DEVICE_PARTITION_MAX_SUB_DEVICES>();
    std::cout << " | PROPERTIES: ";
    auto partition_properties = dev.getInfo<CL_DEVICE_PARTITION_PROPERTIES>();
    for(auto pp : partition_properties)
    {
        switch(pp)
        {
            case CL_DEVICE_PARTITION_EQUALLY:
                std::cout << "EQUALLY";
                break;
            case CL_DEVICE_PARTITION_BY_COUNTS:
                std::cout << "BY_COUNTS";
                break;
            case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
                std::cout << "BY_AFFINITY_DOMAIN";
                break;
        }
        std::cout << ", ";
    }
    std::cout << " | TYPE: ";
    auto partition_type = dev.getInfo<CL_DEVICE_PARTITION_TYPE>();
    for(auto pt : partition_type)
    {
        std::cout << pt << ", ";
    }
    std::cout << "\n+++++++++++++++++++++++++++++++++++++\n";
}

cl::Device getCL_Device()
{
	cl::Platform platform = getCL_Platform();

	std::vector<cl::Device> devices;

	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
	size_t cur_device = devices.size();

    for (size_t d = 0; d < devices.size(); ++d) {
        printCL_DeviceInfo(devices.at(d));
		if (!devices[d].getInfo<CL_DEVICE_AVAILABLE>()) continue;

		std::string ext = devices[d].getInfo<CL_DEVICE_EXTENSIONS>();

		// Get first available GPU device which supports double precision
		if (ext.find("cl_khr_fp64") == std::string::npos || ext.find("cl_amd_fp64") == std::string::npos)
		{
			cur_device = d;
			break;
		}
	}

	if (cur_device == devices.size())
	{
		throw std::domain_error("GPUs with double precision not found.");
	}
    
	return std::move(devices.at(cur_device));
}

int main()
try
{	
	// Get a CL device
	cl::Device device = getCL_Device();

	// Get a CL context
	cl::Context context = cl::Context(device);

	// Create a command queue
	cl::CommandQueue queue(context, device);

	// Compile OpenCL program for found device
	cl::Program program(context,
		cl::Program::Sources(1, std::make_pair(kernel1.c_str(), kernel1.size())));

	try {
		program.build(std::vector<cl::Device>{ device });
	}
	catch (const cl::Error& e) {
		std::cerr << "CL program compilation error\n" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)
			<< "\n/////////////////////////////////////\n" << kernel1
			<< "\n/////////////////////////////////////\n";
		throw e;
	}

	// Create a kernel with the entry function "entry_point"
	cl::Kernel k1(program, "entry_point");

	// Prepare input data.
	std::vector<double> a(N, 0.1);
	std::vector<double> b(N, 3.0);
	std::vector<double> c(N);

	// Allocate device buffers and transfer input data to device
	cl::Buffer A(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, a.size() * sizeof(double), a.data());
	cl::Buffer B(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, b.size() * sizeof(double), b.data());
	cl::Buffer C(context, CL_MEM_READ_WRITE, c.size() * sizeof(double));

	// Set kernel parameters
	k1.setArg(0, static_cast<cl_ulong>(N));
	k1.setArg(1, A);
	k1.setArg(2, B);
	k1.setArg(3, C);

	// Launch kernel on the compute device
	queue.enqueueNDRangeKernel(k1, cl::NullRange, N, cl::NullRange);

	// Get result back to host
	queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(double), c.data());

	// Check result from a random place, must be 0.001
	srand(time(NULL));
	std::cout << c[rand() % N] << std::endl;

	return 0;
}
catch (const cl::Error &err) {
	std::cerr << "OpenCL error: " << err.what() << "(" << err.err() << ")\n";
	return 1;
}
catch (const std::exception &err) {
	std::cerr << "STD exception: " << err.what() << "\n";
	return 2;
}
catch (...)
{
	std::cerr << "Unknown exception\n";
	return 3;
}