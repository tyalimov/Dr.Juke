#include <wdm.h>
#include <ntstrsafe.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/sort.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/scoped_ptr.h>
#include <EASTL/set.h>
#include <EASTL/map.h>
#include <EASTL/unordered_set.h>
#include <EASTL/unordered_map.h>

#include "kcrt/path.h"

class ThisIsAClass {
public:
	ThisIsAClass() {
		DbgPrint("%s\n", __FUNCTION__);
	}
	virtual ~ThisIsAClass() {
		DbgPrint("%s\n", __FUNCTION__);
	}
public:
	void foo() {
		DbgPrint("%s\n", __FUNCTION__);
	}
};

ThisIsAClass test_global_class;

void DbgPrintS(const char* s) {
	DbgPrint("%s\n", s);
}

void stl_test()
{
	apathy::Path path("C:\\users\\user\\Documents");

	for (const auto& s : path.split())
	{
		DbgPrint("%s\n", s.segment.c_str());
	}
	

	eastl::make_unique<DRIVER_OBJECT>();
	eastl::make_shared<UNICODE_STRING>();
	eastl::scoped_ptr<double> dptr(new double(3.6));

	eastl::set<int> set_test;
	set_test.insert(1);
	set_test.insert(3);
	set_test.insert(5);
	set_test.erase(1);

	eastl::map<int, int> map_test;
	map_test[0] = 1;
	map_test[10] = 11;
	map_test[20] = 12;
	map_test.erase(11);

	eastl::vector<int> vec_test;
	vec_test.push_back(2);
	vec_test.push_back(3);
	vec_test.push_back(1);
	eastl::stable_sort(vec_test.begin(), vec_test.end(), eastl::less<int>());
	for (auto e : vec_test) {
		DbgPrint("%d\n", e);
	}

	eastl::string s;
	s = "This a string";
	s.append("any");
	DbgPrint("%s\n", s.c_str());

	eastl::wstring ws;
	ws = L"wide string";
	ws.clear();

	eastl::unordered_set<float> us_test;
	us_test.insert(333);

	eastl::unordered_map<double, eastl::string> um_test;
	um_test.insert(eastl::make_pair(6.6, "9.9"));
}

NTSTATUS SysMain(PDRIVER_OBJECT DrvObject, PUNICODE_STRING RegPath) {
	UNREFERENCED_PARAMETER(RegPath);

	// NOTE OF PATH:
	// SEPARATOR = '/' !!!
	// BASE PATH = %SystemRoot% !!!

	test_global_class.foo();

	auto p1 = new CLIENT_ID[10];
	delete[] p1;

	DrvObject->DriverUnload = [](DRIVER_OBJECT* DriverObject) {
		UNREFERENCED_PARAMETER(DriverObject);
		DbgPrint("DriverUnload");
	};	

	stl_test();

	return STATUS_SUCCESS;
}
