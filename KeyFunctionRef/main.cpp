#include <iostream>
#include <string>
#include <functional>
#include "functionref.h"


// Tests


struct TestStruct
{
	int i = 5;
	long double ld = 984.498;
	const char *cc = nullptr;
	volatile char vc = 'c';

	void operator()(int in)
	{
		std::cout << in << '\n';
	}

	void printInt(int in)
	{
		std::cout << "void TestStruct::printInt(" << in << ")\n";
	}

	int returnInt(int in)
	{
		std::cout << "int TestStruct::returnInt(" << in << ")\n";
		return in;
	}
};

std::string touchStruct(double d)
{
	std::cout << "Touching struct\nd=" << d << '\n';
	TestStruct ts;
	void (TestStruct::*mp)(int) = nullptr;
	mp = &TestStruct::printInt;
	(ts.*mp)(543);

	auto mfn = std::mem_fn(&TestStruct::printInt);
	mfn(&ts, 54);

	return "done";
}


void x(volatile int &i)
{
	std::cout << "{" << i << "}\n";
}

volatile int state = 0;
inline void fr(functionRef<void(volatile int &)> x)
{
	x(state);
	::state += 10;
	std::cout << "::fr ::state = " << state << '\n';
}

void sayHi()
{
	std::cout << "Hi\n";
}

#ifdef _WIN32
# pragma warning( push )
# pragma warning( disable: 4244 )
#endif
int sayDoubleHi( int a,
	int b,
	double c )
{
	return a + b + c;
}
#ifdef _WIN32
# pragma warning( pop )
#endif

void print_num(int i)
{
	std::cout << i << '\n';
}

void print_num_alternative(int i)
{
	std::cout << i << '\n';
}

void print_numConst(const int i)
{
	std::cout << i << '\n';
}

struct printNum
{
	void operator()(int i) const
	{
		std::cout << i << '\n';
	}
};

struct PrintNumStateful
{
	int mi = 10;
	void operator()(int i) const
	{
		std::cout << i + mi << '\n';
	}
};


class Foo
{
	std::string _prive;
public:
	int _num;
	Foo(int num = 100, const std::string& prive = "YUPIKAYEE")
		: _num(num),
		_prive(prive)
	{}
	void print_add(int i)
	{
		std::cout << _num + i << '\n';
	}
	void print_add_const(int i) const
	{
		std::cout << _num + i + 20 << '\n';
	}
	void print_addvoid()
	{
		std::cout << _prive << '\n';
		std::cout << sayDoubleHi(20, _num, 5.5) << '\n';
	}
};

class VI
{
	int i_;
public:
	virtual ~VI() noexcept
	{
		
	}

	VI() = default;
	VI(int i)
		: i_(i)
	{}

	virtual void callMe(const std::string& str) = 0;
	virtual void ccallMe(const std::string& str) const = 0;

	int getBase() const noexcept
	{
		return i_;
	}

	void setBase(int i)
	{
		i_ = i;
	}
};

class VImpl : public VI
{
	char c_;
public:
	virtual ~VImpl() noexcept
	{

	}

	VImpl(int i = 10, char c = 'a')
		: VI(i),
		c_{ c }
	{

	}

	virtual void callMe(const std::string& str) override
	{
		std::cout << "VImpl::callMe(str), str = " << str << '\n'
			<< "i_=" << getBase() << ",c_=" << c_;
	}
	virtual void ccallMe(const std::string& str) const override
	{
		std::cout << "VImpl::ccallMe(str), str = " << str << '\n'
			<< "i_=" << getBase() << ",c_=" << c_;
	}

	void setC(char c)
	{
		c_ = c;
	}
};


int main()
{
	std::cout << std::boolalpha << '\n';

	std::cout << "\n1." << '\n';
	functionRef<void()> fr1{ sayHi };
	std::cout << typeid(fr1).name() << '\n';
	fr1();
	std::cout << "\n2." << '\n';
	functionRef<int(int, int, double)> frr1{ sayDoubleHi };
	std::cout << typeid(frr1).name() << '\n';
	std::cout << frr1(23, 42, 89.4387) << '\n';

	std::cout << "\n3. copy constructing" << '\n';
	functionRef<int(int, int, double)> fcr1{ frr1 };
	std::cout << typeid(fcr1).name() << '\n';
	std::cout << fcr1(23, 42, 89.4387) << '\n';

	std::cout << "\n4. touchStruct" << '\n';
	functionRef<std::string(double)> fr2{ touchStruct };
	std::cout << typeid(fr2).name() << '\n';
	if (fr2)
		std::cout << fr2(354.89) << '\n';

	std::cout << "\n5. lvalue lambda" << '\n';
	auto lambo = [](char c) -> void
	{
		std::cout << "char=" << c << '\n';
		int i = 20 * static_cast<int>(c);
		std::cout << "int=" << i << '\n';
		sayHi();
	};
	functionRef<void(char)> fr3{ lambo };
	std::cout << typeid(fr3).name() << '\n';
	fr3('k');

	std::cout << "\n6. prvalue lambda" << '\n';
	auto lambdar = []() { std::cout << "whatever\n"; };
	functionRef<void()> fr4{ lambdar };
	fr4();
	std::cout << "\n7. copy constructing from prvalue lambda" << '\n';
	functionRef<void()> frr4{ fr4 };
	frr4();

	[[maybe_unused]]
	volatile int k = 1;
	std::cout << "\n8. passing an implicitly constructed by a lambda functionRef to a function" << '\n';
	std::cout << "should print 11" << '\n';
	fr([&k](volatile auto &y) {
		y += k;
	});

	// haven't given an option to construct from std::reference_wrapper
	//auto& lambdaref = []() { std::cout << "whatever reference\n"; };
	//functionRef<void()> fr5{std::ref(lambdaref)};
	//fr5();

	std::cout << "\n9." << '\n';
	functionRef<int()> invoke_laterr([] { return 42; });
	auto valr = invoke_laterr();
	std::cout << valr << '\n';

	std::cout << "\nstd::is_copy_constructible_v<TestStruct>=" << std::is_copy_constructible_v<TestStruct> << '\n';
	std::cout << "10. implicitly converting functionRef through a non-capturing lambda and executing a member function\n";
	auto memberLambda1 = [](TestStruct& ts, int arg) { ts.printInt(arg); };
	functionRef<void(TestStruct&, int)> frm1{ memberLambda1 };
	TestStruct ts;
	frm1(ts, 30);

	std::cout << "\n11. initialize functionRef from a capturing/stateful lambda, type is 'int(int)'; simple" << '\n';
	functionRef<int(int)> frm2 = [&ts](int arg) { return ts.returnInt(arg); };
	int ret = frm2(100);
	std::cout << "returned value = " << ret << '\n';

	std::cout << "\n12. Copy assigning\n";
	functionRef<void()> fr1_copy;
	fr1_copy = fr1;
	fr1_copy();
	if (fr1_copy == fr1)
		std::cout << "FunctionRefs properly copied\n";
	else if (fr1_copy != fr1)
		std::cout << "FunctionRefs not properly copied\n";

	std::cout << "\n13. Copy constructing\n";
	functionRef<void()> fr1_constr{ fr1 };
	fr1_constr();
	if (fr1_constr == fr1)
		std::cout << "FunctionRefs properly copied\n";
	else if (fr1_constr != fr1)
		std::cout << "FunctionRefs not properly copied\n";

	std::cout << "\n13. Copy construction again from a spectre functionRef\n";
	std::cout << "UB - don't do it, it still works good though" << '\n';
	//functionRef il_cpy{ invoke_laterr };	// constructor invoked - invoke_laterr is dead
	//il_cpy();
	//if (il_cpy == invoke_laterr)
	//	std::cout << "il_cpy equal to invoke_laterr - good\n";
	//if (il_cpy != invoke_laterr)
	//	std::cout << "il_cpy equal to invoke_laterr - good - but this could be UB don't do it again\n";

	std::cout << "\n14. move constructing\n";
	functionRef fr1_move = std::move(fr1);
	fr1_move();
	//fr1();	// probably valid, but not so safe to use - according to the standard

	std::cout << "\n15. move constructing again from a prvalue lambda" << '\n';
	std::cout << "still works but UB" << '\n';
	//functionRef<void()> fr3_move{ std::move(lambdar) };
	//fr3_move();
	//lambdar('A');	// empty! don't uncomment or UB!

	std::cout << "\n16. resetting" << '\n';
	fr1_move.reset();
	if (fr1_move)
		std::cout << "fr1_move not properly reset" << '\n';
	else
		std::cout << "fr1_move properly reset" << '\n';

	std::cout << "\n17. initializing with an empty lambda" << '\n';
	std::cout << "works but don't do it" << '\n';
	functionRef<void()> fempty{ [] {} };
	//fempty();
	if (fempty)
		std::cout << "fempty points to valid data - but in an unspecified state" << '\n';
	else
		std::cout << "fempty nullptr'd" << '\n';

	std::cout << "\n18. initializing with a stateful lambda (should print 20.5)" << '\n';
	int intni = 10;
	auto lamstateful = [&intni](int i, float f)
	{
		std::cout << "stateful lambda captured variable is" << '\n' << "result with supplied argument is =" << static_cast<float>(intni + i + f) << '\n';
	};
	functionRef<void(int, float)> fstateful{ lamstateful };
	fstateful(5, 5.5);
	if (fstateful)
		std::cout << "fstateful still lives" << '\n';
	else
		std::cout << "fstateful doesn't live" << '\n';

	std::cout << "\n19. Creating functionRef without initialization" << '\n';
	functionRef<void()> fref;
	if (!fref)
		std::cout << "fref is null - good" << '\n';
	else
		std::cout << "fref is not null - bad" << '\n';

	std::cout << "\n20. Creating functionRef from nullptr" << '\n';
	functionRef<void()> frefnull{ nullptr };
	if (!frefnull)
		std::cout << "frefnull is null - good" << '\n';
	else
		std::cout << "frefnull is not null - bad" << '\n';

	std::cout << "\n21. Testing equality" << '\n';
	if (fref == frefnull)
		std::cout << "Empty lambda is equal to null lambda - good" << '\n';
	else if (fref != frefnull)
		std::cout << "Empty lambda is not equal to null lambda - bad" << '\n';

	std::cout << "\n22. Storing and calling a free const function - should print -9" << '\n';
	functionRef<void(const int)> f_display = print_numConst;
	f_display(-9);
	
	// Member functions
	std::cout << "\n23. Store a call to a member function - should print 200" << '\n';
	functionRef<void(int)> f_print_add;
	Foo foo{};
	f_print_add.setTarget(foo, &Foo::print_add);
	f_print_add(100);

	std::cout << "should print 125" << '\n';
	functionRef<void(void)> f_print_void;
	f_print_void.setTarget(foo, &Foo::print_addvoid);
	f_print_void();

	std::cout << "\n24. Store a call to a const member function - should print 130" << '\n';
	functionRef<void(int)> f_print_add_const;
	f_print_add_const.setTarget(foo, &Foo::print_add_const);
	f_print_add_const(10);

	std::cout << "\n25. Store a call to a (public) data member - no can't do! - good" << '\n';
	functionRef<int()> f_dm;
	Foo food{ 310 };
	//f_dm.setTarget(food, &Foo::_num);	// prevented at compile time good!
	//std::cout << f_dm() << '\n';

	std::cout << "\n26. Store a call to a free function and bind its argument with std::bind" << '\n';
	std::cout << "should print 31337" << '\n';
	functionRef<void()> f_display_31337 = std::bind(print_num, 31337);
	f_display_31337();

	std::cout << "\n27. store a call to a member function and object and bind its argument with std::placeholders::_1" << '\n';
	std::cout << "should print 120" << '\n';
	using std::placeholders::_1;
	functionRef<void(int)> f_add_display2 = std::bind(&Foo::print_add, foo, _1);
	f_add_display2(20);

	std::cout << "\n28. assign new call to the same function" << '\n';
	std::cout << "should print 20" << '\n';
	f_add_display2 = print_num_alternative;
	f_add_display2(20);

	std::cout << "\n29. store a call to a stateless function object (functor)" << '\n';
	std::cout << "should print 18" << '\n';
	functionRef<void(int)> f_display_obj = printNum();
	f_display_obj(18);

	std::cout << "\n30. store a call to a stateful function object (functor)" << '\n';
	std::cout << "should print 27" << '\n';
	functionRef<void(int)> f_display_obj2 = PrintNumStateful();
	f_display_obj2(17);

	std::cout << "\n31. store a functor directly and invoke its operator()" << '\n';
	std::cout << "should print 300" << '\n';
	PrintNumStateful pns{ 100 };
	functionRef<void(int)> f_display_obj3{ pns };
	f_display_obj3(200);

	std::cout << "\n32. Swapping - should print 300 again" << '\n';
	functionRef<void(int)> f_swapped;
	f_swapped.swap(f_display_obj3);
	f_swapped(200);

	if (!f_display_obj3)
		std::cout << "swapped object f_display_obj3 nulled - good" << '\n';
	else
		std::cout << "swapped object f_display_obj3 still lives - bad" << '\n';

	std::cout << "\n33. Calling virtual member function" << '\n';
	VImpl vimpl;
	functionRef<void(std::string)> fv;
	fv.setTarget(vimpl, &VImpl::callMe);
	fv("Hello from virtual member");

	vimpl.setBase(100);
	vimpl.setC('h');
	std::cout << "\n33. Retargeting to const virtual member function" << '\n';
	fv.setTarget(vimpl, &VImpl::ccallMe);
	fv("const virtual access too!?");

	std::cout << '\n' << '\n';
	std::cout << "sizeof(functionRef<void()>)=" << sizeof(functionRef<void()>) << '\n';
	std::cout << "alignof(functionRef<void()>)=" << alignof(functionRef<void()>) << '\n';
	std::cout << "sizeof(functionRef<int(std::string, int, int, double, long double, std::string)>="
		<< sizeof(functionRef<int(std::string, int, int, double, long double, std::string)>) << '\n';
	std::cout << "alignof(functionRef<int(std::string, int, int, double, long double, std::string)>)="
		<< alignof(functionRef<int(std::string, int, int, double, long double, std::string)>) << '\n';

	std::cout << "echo %errorlevel% should return 11" << '\n';

	std::system( "pause" );
	return state == 11 ? EXIT_SUCCESS : EXIT_FAILURE;
}
