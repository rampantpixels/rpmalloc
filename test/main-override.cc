
#if defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <rpmalloc.h>
#include <thread.h>
#include <test.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define pointer_offset(ptr, ofs) (void*)((char*)(ptr) + (ptrdiff_t)(ofs))
#define pointer_diff(first, second) (ptrdiff_t)((const char*)(first) - (const char*)(second))

static size_t _hardware_threads;

static void
test_initialize(void);

static int
test_fail(const char* reason) {
	fprintf(stderr, "FAIL: %s\n", reason);
	return -1;
}

static int
test_alloc(void) {
	void* p = malloc(371);
	if (!p)
		return test_fail("malloc failed");
	if ((rpmalloc_usable_size(p) < 371) || (rpmalloc_usable_size(p) > (371 + 16)))
		return test_fail("usable size invalid");
	rpfree(p);

	p = new int;
	if (!p)
		return test_fail("new failed");
	if (rpmalloc_usable_size(p) != 16)
		return test_fail("usable size invalid");
	delete p;

	p = new int[16];
	if (!p)
		return test_fail("new[] failed");
	if (rpmalloc_usable_size(p) != 16*sizeof(int))
		return test_fail("usable size invalid");
	delete[] p;

	printf("Allocation tests passed\n");
	return 0;
}

static int
test_free(void) {
	free(rpmalloc(371));
	free(new int);
#ifdef _WIN32
	free(new int[16]);
#else
	cfree(new int[16]);
#endif
	printf("Free tests passed\n");
	return 0;	
}

int
test_run(int argc, char** argv) {
	(void)sizeof(argc);
	(void)sizeof(argv);
	test_initialize();
	if (test_alloc())
		return -1;
	if (test_free())
		return -1;
	printf("All tests passed\n");
	return 0;
}

#if (defined(__APPLE__) && __APPLE__)
#  include <TargetConditionals.h>
#  if defined(__IPHONE__) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE) || (defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR)
#    define NO_MAIN 1
#  endif
#elif (defined(__linux__) || defined(__linux))
#  include <sched.h>
#endif

#if !defined(NO_MAIN)

int
main(int argc, char** argv) {
	return test_run(argc, argv);
}

#endif

#ifdef _WIN32
#include <Windows.h>

static void
test_initialize(void) {
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);
	_hardware_threads = (size_t)system_info.dwNumberOfProcessors;
}

#elif (defined(__linux__) || defined(__linux))

static void
test_initialize(void) {
	cpu_set_t prevmask, testmask;
	CPU_ZERO(&prevmask);
	CPU_ZERO(&testmask);
	sched_getaffinity(0, sizeof(prevmask), &prevmask);     //Get current mask
	sched_setaffinity(0, sizeof(testmask), &testmask);     //Set zero mask
	sched_getaffinity(0, sizeof(testmask), &testmask);     //Get mask for all CPUs
	sched_setaffinity(0, sizeof(prevmask), &prevmask);     //Reset current mask
	int num = CPU_COUNT(&testmask);
	_hardware_threads = (size_t)(num > 1 ? num : 1);
}

#else

static void
test_initialize(void) {
	_hardware_threads = 1;
}

#endif
