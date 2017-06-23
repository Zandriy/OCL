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
kernel void main(
       ulong n,
       global const double *a,
       global const double *b,
       global double *c
       )
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
char* CLver_str[] {"OpenCL 1.0", "OpenCL 1.1", "OpenCL 1.2", "OpenCL 2.0", "OpenCL 2.1", "OpenCL 2.2"};

CLver getCLver(const std::string& ver)
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

cl::Platform cl_platform()
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	size_t cur_platform = platforms.size();

	if (!cur_platform)
	{
		throw std::domain_error("OpenCL platforms aren't found.");
	}

	std::string info;

	for (size_t p = 0; p < platforms.size(); ++p)
	{
		std::cout << "\n=====================================\n";
		if (platforms[p].getInfo(CL_PLATFORM_PROFILE, &info) == CL_SUCCESS)
			std::cout << "PROFILE: " << info << "\n";
		if (platforms[p].getInfo(CL_PLATFORM_VERSION, &info) == CL_SUCCESS)
			std::cout << "VERSION: " << info << "\n";
		if (getCLver(info) == v120 || (getCLver(info) >= v120 && cur_platform == platforms.size()))
		{
			cur_platform = p;
		}
		if (platforms[p].getInfo(CL_PLATFORM_NAME, &info) == CL_SUCCESS)
			std::cout << "NAME: " << info << "\n";
		if (platforms[p].getInfo(CL_PLATFORM_VENDOR, &info) == CL_SUCCESS)
			std::cout << "VENDOR: " << info << "\n";
		if (platforms[p].getInfo(CL_PLATFORM_EXTENSIONS, &info) == CL_SUCCESS)
			std::cout << "EXTENSIONS: " << info << "\n";
		std::cout << "=====================================\n";
	}

	if (cur_platform == platforms.size())
	{
		throw std::domain_error("OpenCL 1.2 platform is not found.");
	}

	return std::move(platforms.at(cur_platform));
}

cl::Device cl_device()
{
	cl::Platform platform = cl_platform();

	std::vector<cl::Device> devices;

	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
	size_t cur_device = devices.size();

	for (size_t d = 0; d < devices.size(); ++d) {
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
	
	std::cout << "\n+++++++++++++++++++++++++++++++++++++\n";
	std::cout << devices.at(cur_device).getInfo<CL_DEVICE_NAME>();
	std::cout << "\n+++++++++++++++++++++++++++++++++++++\n";

	return std::move(devices.at(cur_device));
}

int main()
try
{	
	// Get a CL device
	cl::Device device = cl_device();

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

	// Create a kernel with the enter point "main"
	cl::Kernel k1(program, "main");

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