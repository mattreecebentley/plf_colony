// NOTE: This test suite throws exceptions to test catch handling. You might see this show up in the debug output when you run it in MSVC. It doesn't mean something has gone wrong.

#define PLF_COLONY_TEST_DEBUG

#include "plf_tools.h"

#include <numeric> // std::accumulate
#include <functional> // std::greater, std::bind2nd
#include <vector> // range-insert testing
#include <algorithm> // std::find
#include <cstdio> // log redirection, printf
#include <cstdlib> // abort
#include <string> // for_each non-trivial type testing

#ifdef PLF_MOVE_SEMANTICS_SUPPORT
	#include <utility> // std::move
#endif

#ifdef PLF_CPP20_SUPPORT
	#include <ranges>
#endif


#include "plf_colony.h"



void message(const char *message_text)
{
	printf("%s\n", message_text);
}


void title1(const char *title_text)
{
	printf("\n\n\n*** %s ***\n", title_text);
	printf("===========================================\n\n\n");
}

void title2(const char *title_text)
{
	printf("\n\n--- %s ---\n\n", title_text);
}


void failpass(const char *test_type, bool condition)
{
	printf("%s: ", test_type);

	if (condition)
	{
		printf("Pass\n");
	}
	else
	{
		printf("Fail\n");
		getchar();
		abort();
	}
}



#ifdef PLF_VARIADICS_SUPPORT
	struct perfect_forwarding_test
	{
		const bool success;

		perfect_forwarding_test(int&& /*perfect1*/, int& perfect2)
			: success(true)
		{
			perfect2 = 1;
		}

		template <typename T, typename U>
		perfect_forwarding_test(T&& /*imperfect1*/, U&& /*imperfect2*/)
			: success(false)
		{}
	};



	class non_copyable_type
	{
	private:
		int i;
	public:
		non_copyable_type(const non_copyable_type&); // non construction-copyable
		non_copyable_type& operator=(const non_copyable_type&); // non copyable
		non_copyable_type(int a) : i(a) {}
	};

#endif


struct small_struct
{
	double *empty_field_1;
	double unused_number;
	unsigned int empty_field2;
	double *empty_field_3;
	int number;
	unsigned int empty_field4;

	int operator * () const { return number; };
	bool operator == (const small_struct &source) const { return source.number == number; };
	bool operator != (const small_struct &source) const { return source.number != number; };
	bool operator > (const small_struct &source) const { return number > source.number; };
	bool operator < (const small_struct &source) const { return number < source.number; };
	bool operator >= (const small_struct &source) const { return number >= source.number; };
	bool operator <= (const small_struct &source) const { return number <= source.number; };

	small_struct(const unsigned int num): number(num) {};
};



#ifdef PLF_CPP20_SUPPORT
	struct weakly_ordered // Compares by key only, so equivalent values can differ
	{
		int key, extra;
		std::weak_ordering operator <=> (const weakly_ordered &source) const { return key <=> source.key; }
		bool operator == (const weakly_ordered &source) const { return key == source.key; }
		bool operator != (const weakly_ordered &source) const { return key != source.key; }
	};
#endif



struct exceptions_test
{
	int num;

	exceptions_test(const int num2): num(num2) {};

	exceptions_test(const exceptions_test &et)
	{
		if (et.num == 4) throw 123;
		num = et.num;
	};
};





int global_counter = 0;

struct small_struct_non_trivial
{
	double *empty_field_1;
	double unused_number;
	unsigned int empty_field2;
	double *empty_field_3;
	int number;
	unsigned int empty_field4;

	small_struct_non_trivial(const int num) : number(num) {}
	~small_struct_non_trivial() { ++global_counter; }
};



int sort_live_count = 0; // number of sort_counted objects constructed and not yet destroyed

struct sort_counted // small enough for sort() to sort copies of the elements, with non-throwing copy operations and no move operations
{
	int number;

	sort_counted(const int num) : number(num) { ++sort_live_count; }
	sort_counted(const sort_counted &source) PLF_NOEXCEPT : number(source.number) { ++sort_live_count; }
	sort_counted & operator = (const sort_counted &source) PLF_NOEXCEPT { number = source.number; return *this; }
	~sort_counted() { --sort_live_count; }
	bool operator < (const sort_counted &source) const { return number < source.number; }
};



struct sort_throwing_copy // Small, but its copy may throw, so sort() copies it into the sort array with uninitialized_copy where type traits are supported
{
	int number;

	explicit sort_throwing_copy(const int num) PLF_NOEXCEPT : number(num) {}
	sort_throwing_copy(const sort_throwing_copy &source) : number(source.number) { if (source.number < 0) throw 123; }
	sort_throwing_copy & operator = (const sort_throwing_copy &source) { if (source.number < 0) throw 123; number = source.number; return *this; }
	bool operator < (const sort_throwing_copy &source) const { return number < source.number; }
	bool operator == (const sort_throwing_copy &source) const { return number == source.number; }
};



int sort_throwing_live_count = 0; // number of sort_throws_on_copy objects constructed and not yet destroyed
bool sort_throwing_armed = false;

struct sort_throws_on_copy // Small, and its copy throws for the value 500 while sort_throwing_armed is true
{
	int number;

	explicit sort_throws_on_copy(const int num) : number(num) { ++sort_throwing_live_count; }
	sort_throws_on_copy(const sort_throws_on_copy &source) : number(source.number) { if (sort_throwing_armed && source.number == 500) throw 123; ++sort_throwing_live_count; }
	sort_throws_on_copy & operator = (const sort_throws_on_copy &source) { number = source.number; return *this; }
	~sort_throws_on_copy() { --sort_throwing_live_count; }
	bool operator < (const sort_throws_on_copy &source) const { return number < source.number; }
	bool operator == (const sort_throws_on_copy &source) const { return number == source.number; }
};



struct sort_non_trivial // Small and not trivially copyable, with non-throwing copy and move operations, so sort() moves it into the sort array and back where move semantics are supported
{
	int number;

	explicit sort_non_trivial(const int num) PLF_NOEXCEPT : number(num) {}
	sort_non_trivial(const sort_non_trivial &source) PLF_NOEXCEPT : number(source.number) {}
	sort_non_trivial & operator = (const sort_non_trivial &source) PLF_NOEXCEPT { number = source.number; return *this; }

	#ifdef PLF_MOVE_SEMANTICS_SUPPORT
		sort_non_trivial(sort_non_trivial &&source) PLF_NOEXCEPT : number(source.number) { source.number = -1; } // A moved-from element is -1, so an element moved from twice is seen
		sort_non_trivial & operator = (sort_non_trivial &&source) PLF_NOEXCEPT { number = source.number; source.number = -1; return *this; }
	#endif

	bool operator < (const sort_non_trivial &source) const { return number < source.number; }
	bool operator == (const sort_non_trivial &source) const { return number == source.number; }
};



struct sort_by_last_digit // A custom comparator for sort(), comparing the last digit first
{
	bool operator () (const int first, const int second) const { return (first % 10 != second % 10) ? first % 10 < second % 10 : first < second; }
	bool operator () (const std::string &first, const std::string &second) const { return (first[first.size() - 1] != second[second.size() - 1]) ? first[first.size() - 1] < second[second.size() - 1] : first < second; }
};



// sort() must give the same order as sorting a copy of the elements:
template <class colony_type, class comparison_function>
bool sort_matches_sorted_copy(colony_type &source, comparison_function compare)
{
	std::vector<typename colony_type::value_type> expected(source.begin(), source.end());
	std::sort(expected.begin(), expected.end(), compare);
	source.sort(compare);
	return source.size() == expected.size() && std::equal(source.begin(), source.end(), expected.begin());
}





// Checks that max_elements_per_allocation() gives the largest block capacity whose elements and skipfield fit in each amount, or 0 if not even the smallest block fits:
template <class colony_type>
bool max_elements_per_allocation_correct()
{
	const plf::limits hard_limits = colony_type::block_capacity_hard_limits();

	for (size_t amount = 0; amount != 5000; ++amount)
	{
		const size_t capacity = colony_type::max_elements_per_allocation(amount);

		if (capacity == 0)
		{
			if (colony_type::block_allocation_amount(hard_limits.min) <= amount) return false;
		}
		else if (capacity < hard_limits.min || capacity > hard_limits.max || colony_type::block_allocation_amount(capacity) > amount || (capacity != hard_limits.max && colony_type::block_allocation_amount(capacity + 1) <= amount))
		{
			return false;
		}
	}

	return true;
}

// Function objects for the for_each tests. These are at namespace scope, as local types cannot be template arguments before C++11:
struct address_recorder
{
	std::vector<int *> *addresses;
	explicit address_recorder(std::vector<int *> &destination): addresses(&destination) {}
	void operator () (int &element) { addresses->push_back(&element); }
};

struct const_address_recorder
{
	std::vector<const int *> *addresses;
	explicit const_address_recorder(std::vector<const int *> &destination): addresses(&destination) {}
	void operator () (const int &element) { addresses->push_back(&element); }
};

struct element_counter
{
	std::size_t count;
	element_counter(): count(0) {}
	void operator () (const int &) { ++count; }
};

struct stop_counter // Returns false for the element numbered stop, counting from start
{
	std::size_t current, stop;
	stop_counter(const std::size_t start, const std::size_t stop_at): current(start), stop(stop_at) {}
	bool operator () (const int &) { return current++ != stop; }
};

struct constness_checker // For a const colony, only the const overload must be called
{
	bool all_const;
	constness_checker(): all_const(true) {}
	void operator () (const int &) {}
	void operator () (int &) { all_const = false; }
};

struct string_appender
{
	std::string *text;
	explicit string_appender(std::string &destination): text(&destination) {}
	void operator () (std::string &element) { *text += element; }
};

inline int erase_if_key(const int value) { return value; }
inline int erase_if_key(const std::string &value) { return atoi(value.c_str()); }

inline void erase_if_make(const int key, int &value) { value = key; }
inline void erase_if_make(const int key, std::string &value) { char text[16]; sprintf(text, "%d", key); value = text; }

struct erase_if_counting_predicate // Matches keys with key % modulus < matches_below, counting the calls
{
	std::size_t *calls;
	int modulus, matches_below;

	template <class element_type>
	bool operator () (const element_type &value) const
	{
		++*calls;
		return erase_if_key(value) % modulus < matches_below;
	}
};

struct erase_if_every_nth // Stateful: the result depends on the number of calls so far, so erase_if must call the same object throughout, not a copy
{
	int calls, nth;
	std::size_t *total_calls;

	template <class element_type>
	bool operator () (const element_type &)
	{
		++*total_calls;
		return ++calls % nth == 0;
	}
};

// With a stateful predicate matching every nth element (gaps longer than the 32 elements erase_if steps over), erase_if must erase exactly the nth, 2nth... elements in iteration order, and call the predicate once per element
template <class element_type>
bool erase_if_stateful_test(const int prior_erasure, const int nth)
{
	plf::colony<element_type> e_colony(plf::limits(128, 128));
	element_type value;

	for (int counter = 0; counter != 3000; ++counter)
	{
		erase_if_make(counter, value);
		e_colony.insert(value);
	}

	for (typename plf::colony<element_type>::iterator current = e_colony.begin(); current != e_colony.end();)
	{
		const bool erase_element = (prior_erasure == 1) ? (rand() & 63) == 0 : (prior_erasure == 2) ? (rand() & 3) == 0 : false;

		if (erase_element)
		{
			current = e_colony.erase(current);
		}
		else
		{
			++current;
		}
	}

	std::vector<element_type> expected;
	int position = 0;

	for (typename plf::colony<element_type>::iterator current = e_colony.begin(); current != e_colony.end(); ++current)
	{
		if (++position % nth != 0) expected.push_back(*current);
	}

	const std::size_t original_size = e_colony.size();
	std::size_t total_calls = 0;
	erase_if_every_nth predicate = {0, nth, &total_calls};
	const std::size_t erased = erase_if(e_colony, predicate);

	return erased == original_size - expected.size() && e_colony.size() == expected.size() && std::equal(e_colony.begin(), e_colony.end(), expected.begin()) && total_calls == original_size;
}

// erase_if must erase the same elements as a filtered copy does, and call the predicate once per element. Patterns: single matches with short and long gaps, runs of matches, no matches, all matching. Prior erasures: none, few (the word scan in for_each_while), 1 in 4, in every other block only, and sparse with range erasures
template <class element_type>
void erase_if_pattern_test(bool &erase_if_matches, bool &predicate_called_once)
{
	const int patterns[][2] = {{2, 1}, {10, 1}, {33, 1}, {100, 1}, {1000, 3}, {100, 40}, {5000, 1}, {7, 0}, {3, 3}};

	for (int prior_erasure = 0; prior_erasure != 5; ++prior_erasure)
	{
		for (std::size_t pattern = 0; pattern != sizeof(patterns) / sizeof(patterns[0]); ++pattern)
		{
			plf::colony<element_type> e_colony(plf::limits(128, 128));
			element_type value;

			for (int counter = 0; counter != 3000; ++counter)
			{
				erase_if_make((rand() & 1) ? counter : rand(), value);
				e_colony.insert(value);
			}

			int index = 0;

			for (typename plf::colony<element_type>::iterator current = e_colony.begin(); current != e_colony.end(); ++index)
			{
				bool erase_element;

				switch (prior_erasure)
				{
					case 1: erase_element = (rand() & 63) == 0; break;
					case 2: erase_element = (rand() & 3) == 0; break;
					case 3: erase_element = (rand() & 3) == 0 && (index / 128) % 2 == 1; break;
					case 4: erase_element = (rand() & 7) != 0; break;
					default: erase_element = false;
				}

				if (erase_element)
				{
					current = e_colony.erase(current);
				}
				else
				{
					++current;
				}
			}

			if (prior_erasure == 4)
			{
				for (int counter = 0; counter != 20 && e_colony.size() > 40; ++counter) // Range erasures, which join skipblocks
				{
					typename plf::colony<element_type>::iterator first = e_colony.begin(), last;
					advance(first, rand() % static_cast<int>(e_colony.size() - 20));
					last = first;
					advance(last, 1 + rand() % 8);
					e_colony.erase(first, last);
				}
			}

			std::vector<element_type> expected;

			for (typename plf::colony<element_type>::iterator current = e_colony.begin(); current != e_colony.end(); ++current)
			{
				if (erase_if_key(*current) % patterns[pattern][0] >= patterns[pattern][1]) expected.push_back(*current);
			}

			const std::size_t original_size = e_colony.size();
			std::size_t calls = 0;
			const erase_if_counting_predicate predicate = {&calls, patterns[pattern][0], patterns[pattern][1]};
			const std::size_t erased = erase_if(e_colony, predicate);

			if (erased != original_size - expected.size() || e_colony.size() != expected.size() || !std::equal(e_colony.begin(), e_colony.end(), expected.begin()))
			{
				erase_if_matches = false;
			}

			if (calls != original_size)
			{
				predicate_called_once = false;
			}
		}
	}
}

#ifdef PLF_MOVE_SEMANTICS_SUPPORT
	struct unique_ptr_pointee_less
	{
		bool operator () (const std::unique_ptr<int> &left, const std::unique_ptr<int> &right) const { return *left < *right; }
	};

	struct move_only_counter
	{
		std::unique_ptr<std::size_t> count;
		move_only_counter(): count(new std::size_t(0)) {}
		bool operator () (int &) { ++*count; return true; }
	};
#endif


// Element type for the destruction tests. It counts live elements and destructions of objects which are not alive, and can throw from its copy constructor and copy assignment:
int destruction_live_count = 0, destruction_bad_count = 0, destruction_throw_countdown = 0;

template <int padding>
struct live_counter // padding 1 gives an 8-bit skipfield, padding 24 a 16-bit skipfield
{
	int alive;
	char data[padding];

	live_counter(): alive(12345) { ++destruction_live_count; }
	live_counter(const live_counter &): alive(12345) { check_throw(); ++destruction_live_count; } // check_throw() is called before counting, so that a throwing copy is not counted
	live_counter & operator = (const live_counter &) { check_throw(); return *this; }

	~live_counter()
	{
		if (alive != 12345) ++destruction_bad_count;
		alive = 0;
		--destruction_live_count;
	}

	static void check_throw()
	{
		if (destruction_throw_countdown != 0 && --destruction_throw_countdown == 0) throw 123;
	}
};



// clear(), range erase, assign() to fewer elements (also with a throwing copy) and the destructor must destroy each element exactly once, in blocks with and without erasures, and in a back block which is also the first block. Pass 0 has no erasures, pass 1 erasures in every other block only, pass 2 is sparse, pass 3 has erasures in runs of 8:
template <int padding>
void destruction_tests()
{
	typedef live_counter<padding> element_type;
	typedef plf::colony<element_type> colony_type;
	bool clear_destroys = true, range_erase_destroys = true, assign_destroys = true, throwing_assign_destroys = true, fill_assign_destroys = true, destructor_destroys = true;

	for (int pass = 0; pass != 4; ++pass)
	{
		for (int operation = 0; operation != 6; ++operation)
		{
			for (int repeat = 0; repeat != 10; ++repeat)
			{
				{
					colony_type d_colony(plf::limits(64, 64));
					const int total = (repeat % 2 == 0) ? 2000 : 1 + rand() % 150; // Also a single block or two, where the back block is the first block

					for (int counter = 0; counter != total; ++counter)
					{
						d_colony.insert(element_type());
					}

					int index = 0;

					for (typename colony_type::iterator current = d_colony.begin(); current != d_colony.end(); ++index)
					{
						bool erase;

						switch (pass)
						{
							case 0: erase = false; break;
							case 1: erase = (index / 64) % 2 == 1 && (rand() & 3) == 0; break;
							case 2: erase = (rand() & 7) != 0; break;
							default: erase = (index / 8) % 3 == 1; break;
						}

						if (erase)
						{
							current = d_colony.erase(current);
						}
						else
						{
							++current;
						}
					}

					switch (operation)
					{
						case 0:
							d_colony.clear();
							clear_destroys = clear_destroys && destruction_live_count == 0;
							break;
						case 1:
						{
							for (int counter = 0; counter != 10 && d_colony.size() > 2; ++counter)
							{
								const int size = static_cast<int>(d_colony.size());
								const int first_index = rand() % size, last_index = first_index + rand() % (size - first_index + 1);
								typename colony_type::iterator first = d_colony.begin(), last = d_colony.begin();
								advance(first, first_index);
								advance(last, last_index);
								d_colony.erase(first, last);
								range_erase_destroys = range_erase_destroys && destruction_live_count == static_cast<int>(d_colony.size());
							}

							typename colony_type::iterator first = d_colony.begin();
							advance(first, static_cast<int>(d_colony.size() / 2));
							d_colony.erase(first, d_colony.end()); // Up to end()
							range_erase_destroys = range_erase_destroys && destruction_live_count == static_cast<int>(d_colony.size());
							break;
						}
						case 2:
						{
							const std::vector<element_type> fewer(d_colony.size() / 3 + static_cast<unsigned int>(rand() % 64));
							d_colony.assign(fewer.begin(), fewer.end());
							assign_destroys = assign_destroys && d_colony.size() == fewer.size() && destruction_live_count == static_cast<int>(d_colony.size() + fewer.size());
							break;
						}
						case 3:
						{
							const std::vector<element_type> fewer(d_colony.size() / 2);

							if (fewer.empty())
							{
								break;
							}

							destruction_throw_countdown = 1 + rand() % static_cast<int>(fewer.size()); // Throws while assigning to, or constructing in, the colony. The blocks all have 64 slots, and assign() fills them in order, including erased ones

							if ((destruction_throw_countdown - 1) % 64 == 0 && destruction_throw_countdown != 1) // Avoids throwing at the front of a block other than the first, as range_assign() would then leave an empty back block, which is a separate issue
							{
								--destruction_throw_countdown;
							}

							try
							{
								d_colony.assign(fewer.begin(), fewer.end());
								throwing_assign_destroys = false;
							}
							catch (...)
							{
							}

							destruction_throw_countdown = 0;
							throwing_assign_destroys = throwing_assign_destroys && destruction_live_count == static_cast<int>(d_colony.size() + fewer.size());
							break;
						}
						case 4:
						{
							const element_type value;
							d_colony.assign(d_colony.size() / 3 + static_cast<unsigned int>(rand() % 64), value);
							fill_assign_destroys = fill_assign_destroys && destruction_live_count == static_cast<int>(d_colony.size()) + 1;
							break;
						}
						default:
							break;
					}
				}

				if (operation == 5)
				{
					destructor_destroys = destructor_destroys && destruction_live_count == 0;
				}

				destruction_live_count = 0;
			}
		}
	}

	failpass("clear destruction test", clear_destroys);
	failpass("range erase destruction test", range_erase_destroys);
	failpass("assign destruction test", assign_destroys);
	failpass("throwing assign destruction test", throwing_assign_destroys);
	failpass("fill assign destruction test", fill_assign_destroys);
	failpass("destructor destruction test", destructor_destroys);
	failpass("no destruction of erased or destroyed elements test", destruction_bad_count == 0);
}


// A copy which throws at the front of a block other than the first, during fill or range insert or assign, leaves an empty back block. Destroying all elements with clear() or assign() afterwards must not destroy anything in that block (the destructor alone does not handle this yet, so it is not tested here):
template <int padding>
void empty_back_block_tests()
{
	typedef live_counter<padding> element_type;
	typedef plf::colony<element_type> colony_type;
	bool all_threw = true, clear_destroys = true, assign_destroys = true, destructor_destroys = true;
	const int bad_count_before = destruction_bad_count;

	for (int operation = 0; operation != 4; ++operation)
	{
		for (int follow_up = 0; follow_up != 2; ++follow_up)
		{
			{
				const std::vector<element_type> source(20);
				colony_type d_colony(plf::limits(8, 8));

				if (operation == 3) // Range assign to fewer elements assigns to the existing elements, and constructs none
				{
					for (int counter = 0; counter != 32; ++counter)
					{
						d_colony.insert(element_type());
					}
				}

				destruction_throw_countdown = 9; // The 9th copy is to the front of the second block
				bool threw = false;

				try
				{
					switch (operation)
					{
						case 0: d_colony.insert(20, source[0]); break;
						case 1: d_colony.insert(source.begin(), source.end()); break;
						case 2: d_colony.assign(20, source[0]); break;
						default: d_colony.assign(source.begin(), source.end()); break;
					}
				}
				catch (...)
				{
					threw = true;
				}

				destruction_throw_countdown = 0;
				all_threw = all_threw && threw;

				if (follow_up == 0)
				{
					d_colony.clear();
					clear_destroys = clear_destroys && d_colony.empty() && destruction_live_count == static_cast<int>(source.size());
				}
				else
				{
					const std::vector<element_type> fewer(3);
					d_colony.assign(fewer.begin(), fewer.end());
					assign_destroys = assign_destroys && d_colony.size() == fewer.size() && destruction_live_count == static_cast<int>(source.size() + fewer.size() + d_colony.size());
				}
			}

			destructor_destroys = destructor_destroys && destruction_live_count == 0;
			destruction_live_count = 0;
		}
	}

	failpass("empty back block exception test", all_threw);
	failpass("empty back block clear test", clear_destroys);
	failpass("empty back block assign test", assign_destroys);
	failpass("empty back block destructor after clear or assign test", destructor_destroys);
	failpass("empty back block no destruction of destroyed elements test", destruction_bad_count == bad_count_before);
}



// For the copying from a colony tests:

int copy_throws_live_count = 0; // number of copy_throws objects constructed and not yet destroyed

struct copy_throws // the copy constructor and copy assignment throw when copying the element numbered throw_on
{
	int number;
	static int throw_on;

	copy_throws(const int num): number(num) { ++copy_throws_live_count; }
	copy_throws(const copy_throws &source): number(source.number) { if (number == throw_on) throw 123; ++copy_throws_live_count; }
	copy_throws & operator = (const copy_throws &source) { if (source.number == throw_on) throw 123; number = source.number; return *this; }
	~copy_throws() { --copy_throws_live_count; }
	bool operator == (const copy_throws &source) const { return number == source.number; }
	bool operator < (const copy_throws &source) const { return number < source.number; }
};

int copy_throws::throw_on = -2; // -2 for none, as -1 is used for destination elements



template <class element_type>
struct copy_test_value // Makes the element numbered number
{
	static element_type make(const int number) { return element_type(number); }
	static int number(const element_type &element) { return static_cast<int>(element); }
};

template <>
struct copy_test_value<std::string> // Long enough not to fit in the small string buffer, so that leaks and double destruction show up under the address sanitizer
{
	static std::string make(int number)
	{
		std::string text("copy test string number ");
		const std::size_t digits_start = text.size();

		do
		{
			text.insert(text.begin() + static_cast<std::ptrdiff_t>(digits_start), static_cast<char>('0' + number % 10));
		} while ((number /= 10) != 0);

		return text;
	}

	static int number(const std::string &element) { return std::atoi(element.c_str() + 24); }
};

template <>
struct copy_test_value<small_struct> // Trivially copyable, with a 16-bit skipfield
{
	static small_struct make(const int number) { return small_struct(static_cast<unsigned int>(number)); }
	static int number(const small_struct &element) { return element.number; }
};

template <>
struct copy_test_value<copy_throws>
{
	static copy_throws make(const int number) { return copy_throws(number); }
	static int number(const copy_throws &element) { return element.number; }
};



// Fills source with 3000 elements numbered from 0, then erases with one of these patterns: 0 no erasures, 1 erasures in every other block only, 2 one in 32 erased (few erasures per block), 3 one in 4, 4 sparse, 5 erasures in runs of 50
template <class colony_type>
void fill_and_erase(colony_type &source, const int pattern)
{
	typedef typename colony_type::value_type element_type;

	for (int counter = 0; counter != 3000; ++counter)
	{
		source.insert(copy_test_value<element_type>::make(counter));
	}

	int index = 0;
	bool erase_run = false;

	for (typename colony_type::iterator current = source.begin(); current != source.end(); ++index)
	{
		bool erase = false;

		switch (pattern)
		{
			case 1: erase = (index / 128) % 2 == 1 && (rand() & 3) == 0; break;
			case 2: erase = (rand() & 31) == 0; break;
			case 3: erase = (rand() & 3) == 0; break;
			case 4: erase = (rand() & 7) != 0; break;
			case 5: if (index % 50 == 0) erase_run = (rand() & 3) == 0; erase = erase_run; break;
			default: break;
		}

		if (erase)
		{
			current = source.erase(current);
		}
		else
		{
			++current;
		}
	}
}



// Destinations for range insert and assign: blocks of other sizes than the source's, so that fills start and end part way through source blocks, and erased slots, so that some of the insert fills them
template <class colony_type>
void make_destination(colony_type &destination)
{
	typedef typename colony_type::value_type element_type;

	for (int counter = rand() % 100; counter != 0; --counter)
	{
		destination.insert(copy_test_value<element_type>::make(-1));
	}

	for (typename colony_type::iterator current = destination.begin(); current != destination.end();)
	{
		if ((rand() & 3) == 0)
		{
			current = destination.erase(current);
		}
		else
		{
			++current;
		}
	}
}



template <class colony_type>
std::vector<int> element_numbers(const colony_type &source)
{
	std::vector<int> numbers;

	for (typename colony_type::const_iterator current = source.begin(); current != source.end(); ++current)
	{
		numbers.push_back(copy_test_value<typename colony_type::value_type>::number(*current));
	}

	return numbers;
}



template <class colony_type>
bool size_matches(const colony_type &source)
{
	return source.size() == static_cast<typename colony_type::size_type>(std::distance(source.begin(), source.end())) && source.empty() == (source.size() == 0);
}



struct copy_test_results
{
	bool copies, allocator_copies, inserts, range_constructions, assigns, empty_assigns;
	copy_test_results(): copies(true), allocator_copies(true), inserts(true), range_constructions(true), assigns(true), empty_assigns(true) {}
};



// The copy constructor, and range insert, range construction and assign from colony iterators, must copy the same elements, in the same order, as iterating does:
template <class element_type>
void copy_from_colony_test(const int pattern, const int limits, const int ranges, copy_test_results &results)
{
	typedef plf::colony<element_type> colony_type;
	colony_type source;

	if (limits == 1)
	{
		source.reshape(plf::limits(50, 70));
	}

	fill_and_erase(source, pattern);

	const colony_type &const_source = source;
	const std::vector<int> expected = element_numbers(const_source);

	const colony_type copy(const_source);
	results.copies = results.copies && size_matches(copy) && element_numbers(copy) == expected;

	const colony_type allocator_copy(const_source, const_source.get_allocator());
	results.allocator_copies = results.allocator_copies && size_matches(allocator_copy) && element_numbers(allocator_copy) == expected;

	for (int counter = 0; counter != ranges; ++counter)
	{
		const int size = static_cast<int>(expected.size());
		const int first = rand() % (size + 1), last = first + rand() % (size - first + 1);
		const std::vector<int> expected_range(expected.begin() + first, expected.begin() + last);
		colony_type destination(plf::limits(static_cast<std::size_t>(10 + rand() % 200), 255));
		colony_type range_constructed, empty_destination;
		make_destination(destination);
		std::vector<int> expected_after = element_numbers(destination);
		expected_after.insert(expected_after.end(), expected_range.begin(), expected_range.end());

		if ((counter & 1) == 0) // const_iterators
		{
			const typename colony_type::const_iterator first_it = next(const_source.cbegin(), first), last_it = next(const_source.cbegin(), last);
			destination.insert(first_it, last_it);
			colony_type(first_it, last_it).swap(range_constructed);
			empty_destination.assign(first_it, last_it);
		}
		else // iterators
		{
			const typename colony_type::iterator first_it = next(source.begin(), first), last_it = next(source.begin(), last);
			destination.insert(first_it, last_it);
			colony_type(first_it, last_it).swap(range_constructed);
			empty_destination.assign(first_it, last_it);
		}

		std::vector<int> after = element_numbers(destination);
		std::sort(after.begin(), after.end());
		std::sort(expected_after.begin(), expected_after.end());
		results.inserts = results.inserts && size_matches(destination) && after == expected_after;
		results.range_constructions = results.range_constructions && size_matches(range_constructed) && element_numbers(range_constructed) == expected_range;
		results.empty_assigns = results.empty_assigns && size_matches(empty_destination) && element_numbers(empty_destination) == expected_range;

		destination.assign(next(const_source.cbegin(), first), next(const_source.cbegin(), last));
		results.assigns = results.assigns && size_matches(destination) && element_numbers(destination) == expected_range;
	}
}



// Fills which end exactly on the first or the last element of a source block, and start on either: copies [begin, p], [p, end) and [p, p] for each such element p, by range construction, assign into an empty colony and range insert.
// Patterns: 0 no erasures (plain arrays), 1 one element erased in the middle of each block (the visit() path), 2 every third element erased except the first and last of each block (the skipfield path):
bool block_boundary_fill_test(const int pattern)
{
	plf::colony<int> source(plf::limits(50, 50));

	for (int counter = 0; counter != 500; ++counter)
	{
		source.insert(counter);
	}

	for (plf::colony<int>::iterator current = source.begin(); current != source.end();)
	{
		const int position = *current % 50;

		if ((pattern == 1 && position == 25) || (pattern == 2 && position != 0 && position != 49 && position % 3 == 0))
		{
			current = source.erase(current);
		}
		else
		{
			++current;
		}
	}

	const std::vector<int> expected(source.begin(), source.end());
	bool passed = true;
	int index = 0;

	for (plf::colony<int>::const_iterator current = source.cbegin(); current != source.cend(); ++current, ++index)
	{
		if (*current % 50 != 0 && *current % 50 != 49) continue;

		const plf::colony<int>::const_iterator after = next(current, 1);
		const plf::colony<int>::const_iterator firsts[3] = {source.cbegin(), current, current}, lasts[3] = {after, source.cend(), after};
		const int first_indexes[3] = {0, index, index}, last_indexes[3] = {index + 1, static_cast<int>(expected.size()), index + 1};

		for (int range = 0; range != 3; ++range)
		{
			const std::vector<int> expected_range(expected.begin() + first_indexes[range], expected.begin() + last_indexes[range]);
			const plf::colony<int> range_constructed(firsts[range], lasts[range]);
			plf::colony<int> empty_destination, destination(plf::limits(10, 10));
			empty_destination.assign(firsts[range], lasts[range]);

			for (int counter = 0; counter != 7; ++counter)
			{
				destination.insert(-1);
			}

			destination.insert(firsts[range], lasts[range]);
			std::vector<int> expected_after(7, -1), after2 = element_numbers(destination);
			expected_after.insert(expected_after.end(), expected_range.begin(), expected_range.end());
			std::sort(after2.begin(), after2.end());
			std::sort(expected_after.begin(), expected_after.end());

			passed = size_matches(range_constructed) && element_numbers(range_constructed) == expected_range && size_matches(empty_destination) && element_numbers(empty_destination) == expected_range && size_matches(destination) && after2 == expected_after;
		}
	}

	return passed;
}



// After a copy has thrown, the colony must still work: erases its first element, inserts five more, and checks the result against expected (its elements before this):
template <class colony_type>
bool still_works(colony_type &destination, std::vector<int> expected)
{
	if (!destination.empty())
	{
		expected.erase(std::find(expected.begin(), expected.end(), copy_test_value<typename colony_type::value_type>::number(*destination.begin())));
		destination.erase(destination.begin());
	}

	for (int counter = 0; counter != 5; ++counter)
	{
		destination.insert(copy_test_value<typename colony_type::value_type>::make(-3));
		expected.push_back(-3);
	}

	std::vector<int> numbers = element_numbers(destination);
	std::sort(numbers.begin(), numbers.end());
	std::sort(expected.begin(), expected.end());
	return size_matches(destination) && numbers == expected;
}



// When copying an element throws part-way through a range insert into a colony of full blocks, or an assign into an empty colony, the elements copied before it must remain, with the right size(), and none may be leaked or destroyed twice.
// The element which throws is in the first block the fill uses, as upstream does not recover correctly from an exception on the first element of a later block (an empty back block is left, and destroying the colony reads past it),
// nor while filling the rest of a partly-full back block (size() is too large), so the destination's blocks are full:
bool copy_from_colony_throw_test(const int pattern, const int limits)
{
	typedef plf::colony<copy_throws> colony_type;
	bool passed = true;

	{
		colony_type source;

		if (limits == 1)
		{
			source.reshape(plf::limits(50, 70));
		}

		fill_and_erase(source, pattern);
		const std::vector<int> expected = element_numbers(source);
		const int size = static_cast<int>(expected.size());

		for (int counter = 0; counter != 4 && size > 1; ++counter)
		{
			const int first = rand() % (size - 1), throw_index = std::min(first + 1 + rand() % 254, size - 1), last = throw_index + 1 + rand() % (size - throw_index);
			const std::vector<int> expected_range(expected.begin() + first, expected.begin() + throw_index); // The elements before the one which throws
			const colony_type::const_iterator first_it = next(source.cbegin(), first), last_it = next(source.cbegin(), last);
			bool threw = false;

			{
				colony_type destination(plf::limits(255, 255));

				for (int counter2 = 255 * (rand() % 3); counter2 != 0; --counter2)
				{
					destination.insert(copy_throws(-1));
				}

				std::vector<int> expected_after = element_numbers(destination);
				expected_after.insert(expected_after.end(), expected_range.begin(), expected_range.end());

				copy_throws::throw_on = expected[static_cast<std::size_t>(throw_index)];

				try
				{
					destination.insert(first_it, last_it);
				}
				catch (...)
				{
					threw = true;
				}

				copy_throws::throw_on = -2;
				passed = passed && threw && size_matches(destination) && element_numbers(destination) == expected_after;
				passed = passed && still_works(destination, expected_after);
			}

			{
				colony_type empty_destination(plf::limits(255, 255));
				copy_throws::throw_on = expected[static_cast<std::size_t>(throw_index)];
				threw = false;

				try
				{
					empty_destination.assign(first_it, last_it);
				}
				catch (...)
				{
					threw = true;
				}

				copy_throws::throw_on = -2;
				passed = passed && threw && size_matches(empty_destination) && element_numbers(empty_destination) == expected_range;
				passed = passed && still_works(empty_destination, expected_range);
			}

			passed = passed && copy_throws_live_count == static_cast<int>(source.size()); // The destinations have been destroyed
		}
	}

	return passed && copy_throws_live_count == 0;
}



int main()
{
	freopen("error.log","w", stderr); // For catching assertion failure info when run outside of a command line prompt

	using namespace std;
	using namespace plf;


	#ifdef PLF_TYPE_TRAITS_SUPPORT
	{
		title2("Trivially copyable iterators tests");

		{
			typedef plf::colony<small_struct_non_trivial> colony_type;
			failpass("Trivially-copyable iterators for non-trivial types", std::is_trivially_copyable<colony_type::iterator>::value);
			failpass("Trivially-copyable const_iterators for non-trivial types", std::is_trivially_copyable<colony_type::const_iterator>::value);

			failpass("Trivially-copyable reverse_iterators for non-trivial types ", std::is_trivially_copyable<colony_type::reverse_iterator>::value);
			failpass("Trivially-copyable reverse_const_iterators for non-trivial types", std::is_trivially_copyable<colony_type::const_reverse_iterator>::value);
		}

		{
			typedef plf::colony<int> colony_type;
			failpass("Trivially-copyable iterators for trivial types", std::is_trivially_copyable<colony_type::iterator>::value);
			failpass("Trivially-copyable const_iterators for trivial types", std::is_trivially_copyable<colony_type::const_iterator>::value);

			failpass("Trivially-copyable reverse_iterators for trivial types", std::is_trivially_copyable<colony_type::reverse_iterator>::value);
			failpass("Trivially-copyable reverse_const_iterators for trivial types", std::is_trivially_copyable<colony_type::const_reverse_iterator>::value);
		}
	}
	#endif


	printf("\nExceptions will be thrown (and caught) by this suite in order to test sub-function exception recovery. Tests loop many times in order to catch edge cases (via random number generation). Press enter to continue.\n");
	getchar();


	for (unsigned int looper = 0; looper != 100; ++looper)
	{
		{
			title1("Colony");
			title2("Test Basics");

			colony<int *> p_colony;

			failpass("Colony empty", p_colony.empty());
			failpass("Distance on empty colony", distance(p_colony.begin(), p_colony.end()) == 0 && distance(p_colony.rbegin(), p_colony.rend()) == 0);

			int ten = 10;
			p_colony.insert(&ten);

			failpass("Colony not-empty", !p_colony.empty());

			title2("Iterator tests");

			failpass("Begin() working", **p_colony.begin() == 10);
			failpass("End() working", p_colony.begin() != p_colony.end());


			p_colony.clear();

			failpass("Begin = End after clear", p_colony.begin() == p_colony.end());

			int twenty = 20;

			for (unsigned int temp = 0; temp != 200; ++temp)
			{
				p_colony.insert(&ten);
				p_colony.insert(&twenty);
			}

			int total = 0, numtotal = 0;

			for(colony<int *>::iterator the_iterator = p_colony.begin(); the_iterator != p_colony.end(); ++the_iterator)
			{
				++total;
				numtotal += **the_iterator;
			}

			failpass("Iteration count test", total == 400);
			failpass("Iterator access test", numtotal == 6000);

			colony<int *>::iterator plus_twenty = p_colony.begin();
			advance(plus_twenty, 20);
			colony<int *>::iterator plus_two_hundred = p_colony.begin();
			advance(plus_two_hundred, 200);

			failpass("Iterator + distance test", distance(p_colony.begin(), plus_twenty) == 20);
			failpass("Iterator - distance test", distance(plus_two_hundred, p_colony.begin()) == -200);

			{
				colony<int> d_colony(1000, 1, plf::limits(20, 20));

				for (colony<int>::iterator current = d_colony.begin(), end = d_colony.end(); current != end;)
				{
					if ((rand() & 7) == 0)
					{
						current = d_colony.erase(current);
					}
					else
					{
						++current;
					}
				}

				int d_size = static_cast<int>(d_colony.size());

				for (int counter = 0; counter != 10000; ++counter)
				{
					const int dist1 = rand() % (d_size - 2), dist2 = rand() % ((d_size - 2) - dist1);
					colony<int>::iterator first = d_colony.begin(), last;
					advance(first, dist1);
					last = first;
					advance(last, dist2);

					const int dist = static_cast<int>(distance(first, last));

					if (dist != dist2)
					{
						printf("positive distance overload fuzz-test failed, real distance = %d, reported distance = %d, counter = %d, suite loop = %d", dist2, dist, counter, looper);
						getchar();
						abort();
					}
				}

				failpass("Positive distance overload fuzz-test", true);


				for (int counter = 0; counter != 10000; ++counter)
				{
					const int dist1 = rand() % (d_size - 2), dist2 = rand() % ((d_size - 2) - dist1);
					colony<int>::iterator first = d_colony.end(), last;
					advance(first, -dist1);
					last = first;
					advance(last, -dist2);

					const int dist = static_cast<int>(distance(last, first));

					if (dist != dist2)
					{
						printf("Negative distance overload fuzz-test failed, real distance = %d, reported distance = %d, counter = %d, suite loop = %d", dist2, dist, counter, looper);
						getchar();
						abort();
					}
				}

				failpass("Negative distance overload fuzz-test", true);



				for (colony<int>::iterator current = d_colony.begin(), end = d_colony.end(); current!= end;)
				{
					if ((rand() & 3) == 0)
					{
						current = d_colony.erase(current);
					}
					else
					{
						++current;
					}
				}

				d_size = static_cast<int>(d_colony.size());


				for (int counter = 0; counter != 10000; ++counter)
				{
					const int dist1 = rand() % (d_size - 2), dist2 = rand() % (d_size - 2);
					colony<int>::iterator first = d_colony.begin(), last = d_colony.begin();
					advance(first, dist1);
					advance(last, dist2);
					if (last > d_colony.end()) last = d_colony.end();

					const int dist = static_cast<int>(distance(first, last));

					if (dist != dist2 - dist1)
					{
						printf("positive/negative distance overload fuzz-test failed, real distance = %d, reported distance = %d, counter = %d, suite loop = %d", dist2, dist, counter, looper);
						getchar();
						abort();
					}
				}

				failpass("Positive/negative distance overload fuzz-test", true);
			}



			for (int pass = 0; pass != 3; ++pass) // for_each and for_each_while must visit the same elements, in the same order, as iteration does. Pass 1 uses word-aligned blocks with erasures in every other block only, pass 2 a sparse colony with long skipblocks
			{
				colony<int> f_colony(plf::limits(static_cast<size_t>(100 + pass * 28), static_cast<size_t>(100 + pass * 28)));

				for (int counter = 0; counter != 2000; ++counter)
				{
					f_colony.insert(counter);
				}

				for (colony<int>::iterator current = f_colony.begin(); current != f_colony.end();)
				{
					const bool erase_element = (pass == 2) ? (rand() & 7) != 0 : (rand() & 3) == 0 && (pass == 0 || (*current / 128) % 2 == 1);

					if (erase_element)
					{
						current = f_colony.erase(current);
					}
					else
					{
						++current;
					}
				}

				if (pass == 2)
				{
					for (int counter = 0; counter != 20 && f_colony.size() > 40; ++counter) // Range erasures, which join skipblocks
					{
						colony<int>::iterator first = f_colony.begin(), last;
						advance(first, rand() % static_cast<int>(f_colony.size() - 20));
						last = first;
						advance(last, 1 + rand() % 8);
						f_colony.erase(first, last);
					}
				}

				std::vector<colony<int>::iterator> positions;
				std::vector<int *> addresses;

				for (colony<int>::iterator current = f_colony.begin(); current != f_colony.end(); ++current)
				{
					positions.push_back(current);
					addresses.push_back(&*current);
				}

				positions.push_back(f_colony.end());

				std::vector<int *> visited;
				for_each(f_colony, address_recorder(visited));
				failpass("for_each test", visited == addresses);

				const colony<int> &const_f_colony = f_colony;
				failpass("for_each const test", for_each(const_f_colony, element_counter()).count == f_colony.size() && for_each(const_f_colony, constness_checker()).all_const && for_each(f_colony.cbegin(), f_colony.cend(), constness_checker()).all_const);

				colony<int> empty_colony;
				failpass("for_each empty test", for_each(empty_colony, element_counter()).count == 0 && for_each_while(empty_colony, stop_counter(0, 1)).first == empty_colony.end());

				bool ranges_match = true, while_matches = true;

				for (int counter = 0; counter != 1000; ++counter)
				{
					std::size_t first = static_cast<std::size_t>(rand()) % positions.size(), last = static_cast<std::size_t>(rand()) % positions.size();
					if (first > last) std::swap(first, last);

					visited.clear();
					for_each(positions[first], positions[last], address_recorder(visited));

					if (visited != std::vector<int *>(addresses.begin() + static_cast<std::ptrdiff_t>(first), addresses.begin() + static_cast<std::ptrdiff_t>(last)))
					{
						ranges_match = false;
					}

					const std::size_t stop = first + static_cast<std::size_t>(rand()) % (last - first + 1); // stop == last means never stop

					if (for_each_while(positions[first], positions[last], stop_counter(first, stop)).first != positions[stop])
					{
						while_matches = false;
					}
				}

				failpass("for_each range test", ranges_match);
				failpass("for_each_while range test", while_matches);

				{ // Whole-colony for_each_while must stop at the element f first returns false for, for both non-const and const colonies
					const std::size_t stop = f_colony.size() / 2;
					const bool stopped_correctly = for_each_while(f_colony, stop_counter(0, stop)).first == positions[stop];
					const colony<int>::const_iterator const_stopped_at = for_each_while(const_f_colony, stop_counter(0, stop)).first;
					failpass("for_each_while whole colony stop test", stopped_correctly && const_stopped_at == colony<int>::const_iterator(positions[stop]));
				}

				{ // const_iterator ranges must visit the same elements as iterator ranges
					bool const_ranges_match = true;

					for (int counter = 0; counter != 100; ++counter)
					{
						std::size_t first = static_cast<std::size_t>(rand()) % positions.size(), last = static_cast<std::size_t>(rand()) % positions.size();
						if (first > last) std::swap(first, last);

						std::vector<const int *> const_visited;
						for_each(colony<int>::const_iterator(positions[first]), colony<int>::const_iterator(positions[last]), const_address_recorder(const_visited));

						if (!std::equal(const_visited.begin(), const_visited.end(), addresses.begin() + static_cast<std::ptrdiff_t>(first)) || const_visited.size() != last - first)
						{
							const_ranges_match = false;
						}
					}

					failpass("for_each const_iterator range test", const_ranges_match);
				}

				#ifdef PLF_MOVE_SEMANTICS_SUPPORT
				{ // Move-only function objects must work, and come back with their state
					const std::size_t while_count = *for_each_while(f_colony.begin(), f_colony.end(), move_only_counter()).second.count;
					const std::size_t whole_count = *for_each(f_colony, move_only_counter()).count;
					failpass("for_each move-only function test", while_count == f_colony.size() && whole_count == f_colony.size());
				}
				#endif
			}

			for (int pass = 0; pass != 2; ++pass) // Non-trivial element type, which also has a 16-bit skipfield. Pass 1 is sparse
			{
				colony<std::string> s_colony;
				char text[16];

				for (int counter = 0; counter != 1000; ++counter)
				{
					sprintf(text, "%d", counter);
					s_colony.insert(std::string(text));
				}

				int counter = 0;

				for (colony<std::string>::iterator current = s_colony.begin(); current != s_colony.end();)
				{
					const bool erase_element = (pass == 0) ? counter % 3 == 0 : (rand() & 7) != 0;
					++counter;

					if (erase_element)
					{
						current = s_colony.erase(current);
					}
					else
					{
						++current;
					}
				}

				std::string iterated, visited;

				for (colony<std::string>::iterator current = s_colony.begin(); current != s_colony.end(); ++current)
				{
					iterated += *current;
				}

				for_each(s_colony, string_appender(visited));
				failpass("for_each non-trivial type test", visited == iterated && !visited.empty());
			}



			#ifdef PLF_CPP20_SUPPORT
				colony<int *>::const_iterator plus_two_hundred_c = plus_two_hundred;
				colony<int *> colony_copy(plus_twenty, plus_two_hundred_c);

				total = 0;

				for(colony<int *>::iterator the_iterator = colony_copy.begin(); the_iterator != colony_copy.end(); ++the_iterator)
				{
					++total;
				}

				failpass("Range-constructor with differing iterator types test", total == 180);
			#endif


			colony<int *>::iterator next_iterator = next(p_colony.begin(), 5);
			colony<int *>::const_iterator prev_iterator = prev(p_colony.cend(), 300);

			failpass("Iterator next test", distance(p_colony.begin(), next_iterator) == 5);
			failpass("Const iterator prev test", distance(p_colony.cend(), prev_iterator) == -300);

			#if defined(__cplusplus) && __cplusplus >= 201402L
				colony<int *>::iterator prev_iterator2 = prev(p_colony.end(), 300);
				failpass("Iterator/Const iterator equality operator test", prev_iterator == prev_iterator2);
			#endif

			prev_iterator = p_colony.begin();
			advance(prev_iterator, 5);
			failpass("Iterator/Const iterator equality operator test 2", prev_iterator == next_iterator);

			colony<int *> p_colony2;
			p_colony2 = p_colony;
			colony<int *> p_colony3(p_colony);
			colony<int *> p_colony4(p_colony2, p_colony2.get_allocator());

			colony<int *>::iterator it1 = p_colony.begin();
			colony<int *>::const_iterator cit(it1);

			failpass("Copy test", p_colony2.size() == 400);
			failpass("Copy construct test", p_colony3.size() == 400);
			failpass("Allocator-extended copy construct test", p_colony4.size() == 400);


			failpass("Equality operator test", p_colony == p_colony2);
			failpass("Equality operator test 2", p_colony2 == p_colony3);

			p_colony2.insert(&ten);

			failpass("Inequality operator test", p_colony2 != p_colony3);

			#ifdef PLF_CPP20_SUPPORT
				failpass("Spaceship operator test", (p_colony2 <=> p_colony3) != 0);

				{ // The comparison category must be the element type's, so colonies of partially and weakly ordered types can be compared:
					colony<double> d_colony1 = {1.0, 2.0}, d_colony2 = {1.0, 3.0}, nan_colony = {std::numeric_limits<double>::quiet_NaN()};
					static_assert(std::is_same_v<decltype(d_colony1 <=> d_colony2), std::partial_ordering>);
					failpass("Spaceship operator partial_ordering test", d_colony1 < d_colony2 && d_colony2 > d_colony1 && d_colony1 <= d_colony1 && (nan_colony <=> nan_colony) == std::partial_ordering::unordered);

					colony<weakly_ordered> w_colony1 = {{1, 0}, {2, 0}}, w_colony2 = {{1, 5}, {2, 6}}, w_colony3 = {{1, 0}, {3, 0}};
					static_assert(std::is_same_v<decltype(w_colony1 <=> w_colony2), std::weak_ordering>);
					failpass("Spaceship operator weak_ordering test", (w_colony1 <=> w_colony2) == std::weak_ordering::equivalent && w_colony1 < w_colony3);

					colony<int> i_colony1 = {1, 2}, i_colony2 = {1, 3};
					static_assert(std::is_same_v<decltype(i_colony1 <=> i_colony2), std::strong_ordering>);
					failpass("Spaceship operator strong_ordering test", i_colony1 < i_colony2 && i_colony2 >= i_colony1);
				}
			#endif

			numtotal = 0;
			total = 0;

			for (colony<int *>::reverse_iterator the_iterator = p_colony.rbegin(); the_iterator != p_colony.rend(); ++the_iterator)
			{
				++total;
				numtotal += **the_iterator;
			}

			failpass("Reverse iteration count test", total == 400);
			failpass("Reverse iterator access test", numtotal == 6000);

			{
				colony<int> numbers;
				numbers.insert(1);
				colony<int>::reverse_iterator rit = numbers.rend(), rit2 = prev(rit, 1), rit3 = numbers.rbegin();
				failpass("Reverse iteration prev/rbegin/rend test", rit2 == rit3);
			}


			colony<int *>::reverse_iterator r_iterator = p_colony.rbegin();
			advance(r_iterator, 50);

			failpass("Reverse iterator advance and distance test", distance(p_colony.rbegin(), r_iterator) == 50);

			colony<int *>::reverse_iterator r_iterator2 = next(r_iterator, 2);

			failpass("Reverse iterator next and distance test", distance(p_colony.rbegin(), r_iterator2) == 52);

			numtotal = 0;
			total = 0;

			for(colony<int *>::iterator the_iterator = p_colony.begin(); the_iterator < p_colony.end(); advance(the_iterator, 2))
			{
				++total;
				numtotal += **the_iterator;
			}

			failpass("Multiple iteration test", total == 200);
			failpass("Multiple iteration access test", numtotal == 2000);

			numtotal = 0;
			total = 0;

			for(colony<int *>::const_iterator the_iterator = p_colony.cbegin(); the_iterator != p_colony.cend(); ++the_iterator)
			{
				++total;
				numtotal += **the_iterator;
			}

			failpass("Const_iterator test", total == 400);
			failpass("Const_iterator access test", numtotal == 6000);


			numtotal = 0;
			total = 0;

			for(colony<int *>::const_reverse_iterator the_iterator = --colony<int *>::const_reverse_iterator(p_colony.crend()); the_iterator != colony<int *>::const_reverse_iterator(p_colony.crbegin()); --the_iterator)
			{
				++total;
				numtotal += **the_iterator;
			}

			failpass("Const_reverse_iterator -- test", total == 399);
			failpass("Const_reverse_iterator -- access test", numtotal == 5980);

			total = 0;

			for(colony<int *>::iterator the_iterator = ++colony<int *>::iterator(p_colony.begin()); the_iterator < p_colony.end(); ++the_iterator)
			{
				++total;
				the_iterator = p_colony.erase(the_iterator);
			}

			failpass("Partial erase iteration test", total == 200);
			failpass("Post-erase size test", p_colony.size() == 200);

			#ifdef PLF_INITIALIZER_LIST_SUPPORT
				{
					colony<int> trim_colony(2000, 10, {200, 200});
					trim_colony.reserve(4000);
					trim_colony.trim_capacity(3000);
					failpass("trim_capacity(n) test", trim_colony.capacity() == 3000);
				}
			#endif

			{ // trim_capacity(n) on an empty colony, where the first block is deallocated and an unused block that had erasures becomes the only block
				colony<int> trim_colony(plf::limits(3, 4));

				for (int counter = 0; counter != 8; ++counter)
				{
					trim_colony.insert(counter); // blocks of 3, 3 and 4 elements
				}

				trim_colony.erase(--trim_colony.end());
				trim_colony.clear();
				trim_colony.trim_capacity(4); // deallocates both 3-element blocks
				trim_colony.insert(1);

				unsigned int count = 0;

				for (colony<int>::iterator current = trim_colony.begin(); current != trim_colony.end(); ++current)
				{
					if (++count == 2) break; // There is only one element
				}

				failpass("trim_capacity(n) after clear test", count == 1 && trim_colony.capacity() == 4);
			}

			const unsigned int temp_capacity = static_cast<unsigned int>(p_colony.capacity());
			p_colony.shrink_to_fit();
			failpass("Shrink_to_fit test", p_colony.capacity() < temp_capacity);
			failpass("Shrink_to_fit test 2", p_colony.capacity() == 200);

			total = 0;

			for(colony<int *>::reverse_iterator the_iterator = p_colony.rbegin(); the_iterator != p_colony.rend();)
			{
				colony<int *>::iterator it = the_iterator.base();
				the_iterator = colony<int *>::reverse_iterator(p_colony.erase(--it));
				++total;
			}

			failpass("Full erase reverse iteration test", total == 200);
			failpass("Post-erase size test", p_colony.size() == 0);

			for (unsigned int temp = 0; temp != 200; ++temp)
			{
				p_colony.insert(&ten);
				p_colony.insert(&twenty);
			}

			total = 0;

			for(colony<int *>::iterator the_iterator = --colony<int *>::iterator(p_colony.end()); the_iterator != p_colony.begin(); --the_iterator)
			{
				++total;
			}

			failpass("Negative iteration test", total == 399);


			total = 0;

			for(colony<int *>::iterator the_iterator = --(colony<int *>::iterator(p_colony.end())); the_iterator > p_colony.begin(); advance(the_iterator, -2))
			{
				++total;
			}

			failpass("Negative multiple iteration test", total == 200);

			#ifdef PLF_MOVE_SEMANTICS_SUPPORT
				p_colony2 = std::move(p_colony);
				failpass("Move test", p_colony2.size() == 400);

				p_colony.insert(&ten);

				failpass("Insert to post-moved-colony test", p_colony.size() == 1);

				colony<int *> p_colony5(p_colony2);
				colony<int *> p_colony6(std::move(p_colony5), p_colony2.get_allocator());

				failpass("Allocator-extended move construct test", p_colony6.size() == 400);
			#else
				p_colony2 = p_colony;
			#endif

			p_colony3 = p_colony2;

			failpass("Copy test 2", p_colony3.size() == 400);

			p_colony2.insert(&ten);

			p_colony2.swap(p_colony3);

			failpass("Swap test", p_colony2.size() == p_colony3.size() - 1);

			std::swap(p_colony2, p_colony3);

			failpass("Swap test 2", p_colony3.size() == p_colony2.size() - 1);

			failpass("max_size() test", p_colony2.max_size() > p_colony2.size());

		}


		{
			title2("Iterator comparison tests");

			colony<int> i_colony;

			for (int temp = 0; temp != 10; ++temp)
			{
				i_colony.insert(temp);
			}

			colony<int>::iterator it1 = i_colony.begin(), it2 = i_colony.begin();

			++it2;
			++it2;
			++it2;

			failpass("Iterator ++ test", *it2 == 3);

			failpass("Iterator > test", it2 > it1);

			failpass("Iterator >= test", it2 >= it1);

			failpass("Iterator < test", it1 < it2);

			failpass("Iterator <= test", it1 <= it2);

			failpass("Iterator != test", it2 != it1);

			#ifdef PLF_CPP20_SUPPORT
				failpass("Iterator <=> test 1", (it2 <=> it1) == std::strong_ordering::greater);

				failpass("Iterator <=> test 2", (it1 <=> it2) == std::strong_ordering::less);

				it1 = it2;

				failpass("Iterator <=> test 3", (it1 <=> it2) == std::strong_ordering::equal);
			#endif
		}


		{
			title2("Insert and Erase tests");

			colony<int> i_colony;

			for (int temp = 0; temp != 500000; ++temp)
			{
				i_colony.insert(temp);
			}


			failpass("Size after insert test", i_colony.size() == 500000);


			colony<int>::iterator found_item = std::find(i_colony.begin(), i_colony.end(), 5000);;

			failpass("std::find iterator test", *found_item == 5000);


			colony<int>::reverse_iterator found_item2 = std::find(i_colony.rbegin(), i_colony.rend(), 5000);;

			failpass("std::find reverse_iterator test", *found_item2 == 5000);


			for (colony<int>::iterator the_iterator = i_colony.begin(); the_iterator != i_colony.end(); ++the_iterator)
			{
				the_iterator = i_colony.erase(the_iterator);
			}

			failpass("Erase alternating test", i_colony.size() == 250000);

			do
			{
				for (colony<int>::iterator the_iterator = i_colony.begin(); the_iterator != i_colony.end();)
				{
					if ((rand() & 7) == 0)
					{
						the_iterator = i_colony.erase(the_iterator);
					}
					else
					{
						++the_iterator;
					}
				}
			} while (!i_colony.empty());

			failpass("Erase randomly till-empty test", i_colony.size() == 0);


			i_colony.reset();
			i_colony.reshape(plf::limits(100, i_colony.block_capacity_limits().max));

			i_colony.insert(30000, 1); // fill-insert 30000 elements

			failpass("Size after reinitialize + fill-insert test", i_colony.size() == 30000);

			unsigned short count2 = 0;

			do
			{
				for (colony<int>::iterator the_iterator = i_colony.begin(); the_iterator != i_colony.end();)
				{
					if ((rand() & 7) == 0)
					{
						the_iterator = i_colony.erase(the_iterator);
						++count2;
					}
					else
					{
						++the_iterator;
					}
				}

			} while (count2 < 15000);

			failpass("Erase randomly till half-empty test", i_colony.size() == 30000u - count2);

			i_colony.insert(count2, 1);

			failpass("Size after reinsert test", i_colony.size() == 30000);




			unsigned int sum = 0;

			for (colony<int>::iterator the_iterator = i_colony.begin(); the_iterator != i_colony.end();)
			{
				if (++sum == 3)
				{
					sum = 0;
					the_iterator = i_colony.erase(the_iterator);
				}
				else
				{
					i_colony.insert(1);
					++the_iterator;
				}
			}

			failpass("Alternating insert/erase test", i_colony.size() == 45001);


			do
			{
				for (colony<int>::iterator the_iterator = i_colony.begin(); the_iterator != i_colony.end();)
				{
					if ((rand() & 3) == 0)
					{
						++the_iterator;
						i_colony.insert(1);
					}
					else
					{
						the_iterator = i_colony.erase(the_iterator);
					}
				}
			} while (!i_colony.empty());;

			failpass("Random insert/erase till empty test", i_colony.size() == 0);


			i_colony.insert(500000, 10);

			failpass("Insert post-erase test", i_colony.size() == 500000);
			colony<int>::iterator it2 = i_colony.begin();
			advance(it2, 250000);


			for (; it2 != i_colony.end();)
			{
				it2 = i_colony.erase(it2);
			}

			failpass("Large multi-increment iterator test", i_colony.size() == 250000);

			i_colony.insert(250000, 10);

			colony<int>::iterator end_iterator = i_colony.end();
			colony<int>::iterator end_iterator2 = i_colony.end();
			advance(end_iterator, -250000);

			for (unsigned int count = 0; count != 250000; ++count, --end_iterator2){}
			failpass("Large multi-decrement iterator test 1", end_iterator == end_iterator2);


			for (colony<int>::iterator the_iterator = i_colony.begin(); the_iterator != end_iterator;)
			{
				the_iterator = i_colony.erase(the_iterator);
			}

			failpass("Large multi-decrement iterator test", i_colony.size() == 250000);



			i_colony.insert(250000, 10);
			int total = 0;

			for (colony<int>::iterator the_iterator = i_colony.begin(); the_iterator != i_colony.end(); ++the_iterator)
			{
				total += *the_iterator;
			}

			failpass("Re-insert post-heavy-erasure test", total == 5000000);


			end_iterator = i_colony.end();
			advance(end_iterator, -50001);
			colony<int>::iterator begin_iterator = i_colony.begin();
			advance(begin_iterator, 300000);

			for (colony<int>::iterator the_iterator = begin_iterator; the_iterator != end_iterator;)
			{
				the_iterator = i_colony.erase(the_iterator);
			}

			failpass("Non-end decrement + erase test", i_colony.size() == 350001);


			i_colony.insert(100000, 10);

			begin_iterator = i_colony.begin();
			advance(begin_iterator, 300001);


			for (colony<int>::iterator the_iterator = begin_iterator; the_iterator != i_colony.end();)
			{
				the_iterator = i_colony.erase(the_iterator);
			}

			failpass("Non-beginning increment + erase test", i_colony.size() == 300001);

			colony<int>::iterator temp_iterator = i_colony.begin();
			advance(temp_iterator, 20); // Advance test 1

			unsigned int index = static_cast<unsigned int>(distance(i_colony.begin(), temp_iterator));
			failpass("Advance + iterator-to-index test", index == 20);

			i_colony.erase(temp_iterator);

			failpass("is_active test 1", i_colony.is_active(temp_iterator) == false);
			failpass("get_iterator test 1", i_colony.get_iterator(&(*temp_iterator)) == i_colony.end());

			temp_iterator = i_colony.begin(); // Check edge-case with advance when erasures present in initial group

			failpass("is_active test 2", i_colony.is_active(temp_iterator) == true);
			failpass("get_iterator test 2", i_colony.get_iterator(&(*temp_iterator)) == temp_iterator);

			advance(temp_iterator, 500);

			index = static_cast<unsigned int>(distance(i_colony.begin(), temp_iterator));

			failpass("Advance + iterator-to-index test", index == 500);

			colony<int>::iterator temp2 = i_colony.get_iterator(&(*temp_iterator));
			colony<int>::const_iterator temp3 = i_colony.get_iterator(const_cast<colony<int>::const_pointer>(&(*temp_iterator)));

			failpass("Pointer-to-iterator test", temp2 != i_colony.end());
			failpass("Const_pointer-to-const_iterator test", temp3 != i_colony.end());

			temp2 = i_colony.begin();
			advance(temp2, 500);

			failpass("Index-to-iterator test", temp2 == temp_iterator);

			{ // distance() to end() when end() is in a back block that has erasures
				colony<int> one_slot_left(plf::limits(3, 3));
				one_slot_left.insert(1);
				one_slot_left.insert(2);
				one_slot_left.erase(one_slot_left.begin()); // _ 2 and one unused slot: end() is at the block's last slot

				colony<int> full(plf::limits(3, 3));
				full.insert(1);
				full.insert(2);
				full.insert(3);
				full.erase(++full.begin()); // 1 _ 3: end() is past the end of the full block

				failpass("Distance to end() in back block test", distance(one_slot_left.begin(), one_slot_left.end()) == 1 && distance(full.begin(), full.end()) == 2);
			}


			for (colony<int>::iterator the_iterator = i_colony.begin(); the_iterator != i_colony.end();)
			{
				the_iterator = i_colony.erase(the_iterator);
			}

			failpass("Total erase test", i_colony.empty());

			// Test get_iterator etc on an empty colony with empty memory blocks retained:
			failpass("is_active test 3", i_colony.is_active(temp_iterator) == false);
  			failpass("get_iterator test 3", i_colony.get_iterator(&(*temp_iterator)) == i_colony.end());

			i_colony.trim_capacity();

			// Test get_iterator etc on a colony with no blocks:
			failpass("is_active test 4", i_colony.is_active(temp_iterator) == false);
  			failpass("get_iterator test 4", i_colony.get_iterator(&(*temp_iterator)) == i_colony.end());

			{ // Range-erase next to an existing skipblock joins the two; every erased element must still be reported as erased
				colony<int> skip_colony;

				for (int i = 0; i != 10; ++i)
				{
					skip_colony.insert(i);
				}

				colony<int>::iterator first = skip_colony.begin();
				advance(first, 3);
				skip_colony.erase(first); // 0 1 2 _ 4 5 6 7 8 9

				first = skip_colony.begin();
				advance(first, 3);
				colony<int>::iterator last = first;
				advance(last, 3);

				const colony<int>::const_iterator erased_iterator = first; // element 4, the first element of the range
				int * const erased_pointer = &(*first);

				skip_colony.erase(first, last); // 0 1 2 _ _ _ _ 7 8 9

				failpass("is_active range-erase join test", skip_colony.is_active(erased_iterator) == false);
				failpass("get_iterator range-erase join test", skip_colony.get_iterator(erased_pointer) == skip_colony.end());
			}

			i_colony.reshape(plf::limits(3, i_colony.block_capacity_limits().max));

			const unsigned int temp_capacity2 = static_cast<unsigned int>(i_colony.capacity());
			i_colony.reserve(100000);
			failpass("Post-reset reserve test", temp_capacity2 != i_colony.capacity());

			i_colony.insert(110000, 1);

			failpass("Post-reserve insert test", i_colony.size() == 110000);

			unsigned int count = 110000;

			for (unsigned int loop1 = 0; loop1 != 50000; ++loop1)
			{
				for (unsigned int loop = 0; loop != 10; ++loop)
				{
					if ((rand() & 7) == 0)
					{
						i_colony.insert(1);
						++count;
					}
				}


				for (colony<int>::iterator the_iterator = i_colony.begin(); the_iterator != i_colony.end();)
				{
					if ((rand() & 7) == 0)
					{
						the_iterator = i_colony.erase(the_iterator);
						--count;
					}
					else
					{
						++the_iterator;
					}
				}
			}

			failpass("Multiple sequential small insert/erase commands test", count == i_colony.size());
		}


		{
			title2("Range-erase tests");

			colony<int> i_colony;

			int counter = 0;

			for (; counter != 1000; ++counter)
			{
				i_colony.insert(counter);
			}


			colony<int>::iterator it1 = i_colony.begin(), it2 = i_colony.begin();

			advance(it1, 500);
			advance(it2, 800);

			i_colony.erase(it1, it2);

			counter = 0;

			for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
			{
				++counter;
			}

			failpass("Simple range-erase test 1", counter == 700 && i_colony.size() == 700);


			it1 = it2 = i_colony.begin();

			advance(it1, 400);
			advance(it2, 500); // This should put it2 past the point of previous erasures

			i_colony.erase(it1, it2);

			counter = 0;

			for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
			{
				++counter;
			}

			failpass("Simple range-erase test 2", counter == 600 && i_colony.size() == 600);



			it2 = it1 = i_colony.begin();

			advance(it1, 4);
			advance(it2, 9); // This should put it2 past the point of previous erasures

			i_colony.erase(it1, it2);

			counter = 0;

			for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
			{
				++counter;
			}

			failpass("Simple range-erase test 3", counter == 595 && i_colony.size() == 595);




			it2 = it1 = i_colony.begin();

			advance(it2, 50);

			i_colony.erase(it1, it2);

			counter = 0;

			for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
			{
				++counter;
			}

			failpass("Range-erase from begin()", counter == 545 && i_colony.size() == 545);




			it1 = i_colony.begin();
			it2 = i_colony.end();

			advance(it1, 345); // Test erasing and validity when it removes the final group in colony
			i_colony.erase(it1, it2);

			counter = 0;

			for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
			{
				++counter;
			}

			failpass("Range-erase to end()", counter == 345 && i_colony.size() == 345);



			i_colony.clear();

			for (counter = 0; counter != 3000; ++counter)
			{
				i_colony.insert(counter);
			}

			for (colony<int>::iterator it = i_colony.begin(); it < i_colony.end(); ++it)
			{
				it = i_colony.erase(it);
			}

			it2 = it1 = i_colony.begin();

			advance(it1, 4);
			advance(it2, 600);
			i_colony.erase(it1, it2);

			counter = 0;

			for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
			{
				++counter;
			}

			failpass("Range-erase with colony already half-erased, alternating erasures", counter == 904 && i_colony.size() == 904);



			i_colony.clear();

			for (counter = 0; counter != 3000; ++counter)
			{
				i_colony.insert(counter);
			}

			for (colony<int>::iterator it = i_colony.begin(); it < i_colony.end(); ++it)
			{
				if ((rand() & 1) == 0)
				{
					it = i_colony.erase(it);
				}
			}

			if (i_colony.size() < 400)
			{
				for (counter = 0; counter != 400; ++counter)
				{
					i_colony.insert(counter);
				}
			}

			it1 = i_colony.begin();
			it2 = i_colony.end();

			advance(it1, 400);
			i_colony.erase(it1, it2);

			counter = 0;

			for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
			{
				++counter;
			}

			failpass("Range-erase with colony already third-erased, randomized erasures", counter == 400 && i_colony.size() == 400);



			unsigned int size, range1, range2, internal_loop_counter;

			for (unsigned int loop_counter = 0; loop_counter != 50; ++loop_counter)
			{
				i_colony.clear();

				for (counter = 0; counter != 1000; ++counter)
				{
					i_colony.insert(counter);
				}

				internal_loop_counter = 0;

				while (!i_colony.empty())
				{
					it2 = it1 = i_colony.begin();

					size = static_cast<unsigned int>(i_colony.size());
					range1 = rand() % size;
					range2 = range1 + 1 + (rand() % (size - range1));
					advance(it1, static_cast<int>(range1));
					advance(it2, static_cast<int>(range2));

					if (it2 > i_colony.end()) it2 = i_colony.end();

					i_colony.erase(it1, it2);

					counter = 0;

					for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
					{
						++counter;
					}

					if (i_colony.size() != static_cast<unsigned int>(counter))
					{
						printf("Fuzz-test range-erase randomly Fail: loop counter: %u, internal_loop_counter: %u.\n", loop_counter, internal_loop_counter);
						getchar();
						abort();
					}

					if (i_colony.size() != i_colony.group_size_sum())
					{
						printf("Fuzz-test range-erase randomly Fail - group_size_sum failure: loop counter: %u, internal_loop_counter: %u, size: %u, group_size_sum: %u.\n", loop_counter, internal_loop_counter, static_cast<unsigned int>(i_colony.size()), static_cast<unsigned int>(i_colony.group_size_sum()));
						getchar();
						abort();
					}

					if (i_colony.size() > 2)
					{ // Test to make sure our stored erased_locations are valid
						i_colony.insert(1);
						i_colony.insert(10);
					}

					++internal_loop_counter;
				}
			}

			failpass("Fuzz-test range-erase randomly until empty", i_colony.size() == 0);



			for (unsigned int loop_counter = 0; loop_counter != 50; ++loop_counter)
			{
				i_colony.clear();
				internal_loop_counter = 0;

				i_colony.insert(10000, 10);

				while (!i_colony.empty())
				{
					it2 = it1 = i_colony.begin();

					size = static_cast<unsigned int>(i_colony.size());
					range1 = rand() % size;
					range2 = range1 + 1 + (rand() % (size - range1));
					advance(it1, static_cast<int>(range1));
					advance(it2, static_cast<int>(range2));
					if (it2 > i_colony.end()) it2 = i_colony.end();

					i_colony.erase(it1, it2);

					counter = 0;

					for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
					{
						++counter;
					}

					if (i_colony.size() != i_colony.group_size_sum())
					{
						printf("Fuzz-test range-erase + fill-insert randomly Fails during erase - group_size_sum failure: loop counter: %u, internal_loop_counter: %u, size: %u, group_size_sum: %u.\n", loop_counter, internal_loop_counter, static_cast<unsigned int>(i_colony.size()), static_cast<unsigned int>(i_colony.group_size_sum()));
						getchar();
						abort();
					}

					if (i_colony.size() != static_cast<unsigned int>(counter))
					{
						printf("Fuzz-test range-erase + fill-insert randomly Fails during erase: loop counter: %u, internal_loop_counter: %u.\n", loop_counter, internal_loop_counter);
						getchar();
						abort();
					}

					if (i_colony.size() > 100)
					{ // Test to make sure our stored erased_locations are valid & fill-insert is functioning properly in these scenarios
						const unsigned int extra_size = rand() & 127;
						i_colony.insert(extra_size, 5);

						if (i_colony.size() != i_colony.group_size_sum())
						{
							printf("Fuzz-test range-erase + fill-insert randomly Fails during insert - group_size_sum failure: loop counter: %u, internal_loop_counter: %u, size: %u, group_size_sum: %u.\n", loop_counter, internal_loop_counter, static_cast<unsigned int>(i_colony.size()), static_cast<unsigned int>(i_colony.group_size_sum()));
							getchar();
							abort();
						}

						if (i_colony.size() != static_cast<unsigned int>(counter) + extra_size)
						{
							printf("Fuzz-test range-erase + fill-insert randomly Fails during fill-insert: loop counter: %u, internal_loop_counter: %u.\n", loop_counter, internal_loop_counter);
							getchar();
							abort();
						}

						counter = 0;

						for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
						{
							++counter;
						}

						if (i_colony.size() != static_cast<unsigned int>(counter))
						{
							printf("Fuzz-test range-erase + fill-insert randomly Fails during counter-test fill-insert: loop counter: %u, internal_loop_counter: %u.\n", loop_counter, internal_loop_counter);
							getchar();
							abort();
						}
					}

					++internal_loop_counter;
				}
			}

			failpass("Fuzz-test range-erase + fill-insert randomly until empty", i_colony.size() == 0);

			i_colony.erase(i_colony.begin(), i_colony.end());

			failpass("Range-erase when colony is empty test (crash test)", i_colony.size() == 0);

			i_colony.insert(10, 1);

			i_colony.erase(i_colony.begin(), i_colony.begin());

			failpass("Range-erase when range is empty test (crash test)", i_colony.size() == 10);
		}



		{
			title1("Non-trivial type tests");

			colony<small_struct_non_trivial> ss_nt;
			colony<small_struct_non_trivial>::iterator ss_it1, ss_it2;

			small_struct_non_trivial ss(5);

			unsigned int size, range1 = 0, range2 = 0, internal_loop_counter;
			int counter;

			ss_nt.insert(10000, ss);

			failpass("Non-trivial type insert test", ss_nt.size() == 10000);


			for (colony<small_struct_non_trivial>::iterator ss_it = ss_nt.begin(); ss_it != ss_nt.end(); ++ss_it)
			{
				ss_it = ss_nt.erase(ss_it);
				++range1;
			}

			failpass("Non-trivial type erase half of all elements", ss_nt.size() == 5000);


			colony<small_struct_non_trivial> ss_nt2;
			ss_nt2.insert(5, ss);

			ss_nt = ss_nt2;

			failpass("Non-trivial type range-assign", ss_nt.size() == 5);



			for (unsigned int loop_counter = 0; loop_counter != 50; ++loop_counter)
			{
				ss_nt.clear();

				for (counter = 0; counter != 1000; ++counter)
				{
					ss_nt.insert(counter);
				}

				internal_loop_counter = 0;

				while (!ss_nt.empty())
				{
					ss_it2 = ss_it1 = ss_nt.begin();

					size = static_cast<unsigned int>(ss_nt.size());
					range1 = rand() % size;
					range2 = range1 + 1 + (rand() % (size - range1));
					advance(ss_it1, static_cast<int>(range1));
					advance(ss_it2, static_cast<int>(range2));
					if (ss_it2 > ss_nt.end()) ss_it2 = ss_nt.end();

					ss_nt.erase(ss_it1, ss_it2);

					counter = 0;

					for (colony<small_struct_non_trivial>::iterator ss_it = ss_nt.begin(); ss_it != ss_nt.end(); ++ss_it)
					{
						++counter;
					}

					if (ss_nt.size() != static_cast<unsigned int>(counter))
					{
						printf("Fuzz-test range-erase randomly Fail: loop counter: %u, internal_loop_counter: %u.\n", loop_counter, internal_loop_counter);
						getchar();
						abort();
					}

					if (ss_nt.size() != ss_nt.group_size_sum())
					{
						printf("Fuzz-test range-erase randomly Fail - group_size_sum failure: loop counter: %u, internal_loop_counter: %u, size: %u, group_size_sum: %u.\n", loop_counter, internal_loop_counter, static_cast<unsigned int>(ss_nt.size()), static_cast<unsigned int>(ss_nt.group_size_sum()));
						getchar();
						abort();
					}

					if (ss_nt.size() > 2)
					{ // Test to make sure our stored erased_locations are valid
						ss_nt.insert(1);
						ss_nt.insert(10);
					}

					++internal_loop_counter;
				}
			}

			failpass("Non-trivial type fuzz-test range-erase randomly until empty", ss_nt.size() == 0);
		}




		{
			title2("Sort tests");

			colony<int> i_colony;

			i_colony.reserve(50000);

			for (unsigned int temp = 0; temp != 50000; ++temp)
			{
				i_colony.insert(rand() & 65535);
			}

			i_colony.sort();

			bool sorted = true;
			int previous = 0;

			for (colony<int>::iterator current = i_colony.begin(); current != i_colony.end(); ++current)
			{
				if (previous > *current)
				{
					sorted = false;
					break;
				}

				previous = *current;
			}

			failpass("Less-than sort test", sorted);

			i_colony.unique();


			bool unique = true;
			previous = -1;

			for (colony<int>::iterator current = i_colony.begin(); current != i_colony.end(); ++current)
			{
				if (previous == *current)
				{
					unique = false;
					break;
				}

				previous = *current;
			}

			failpass("Unique test", unique);

			i_colony.sort(std::greater<int>());

			previous = 65536;

			for (colony<int>::iterator current = i_colony.begin(); current != i_colony.end(); ++current)
			{
				if (previous < *current)
				{
					sorted = false;
					break;
				}

				previous = *current;
			}

			failpass("Greater-than sort test", sorted);


			i_colony.clear();
			i_colony.trim_capacity();
			i_colony.reserve(512);

			for (unsigned int temp = 0; temp != 200; ++temp)
			{
				i_colony.insert(rand() & 65535);
			}

			i_colony.sort();

			previous = 0;

			for (colony<int>::iterator current = i_colony.begin(); current != i_colony.end(); ++current)
			{
				if (previous > *current)
				{
					sorted = false;
					break;
				}

				previous = *current;
			}

			failpass("Less-than sort with allocation optimization 1 test - small types", sorted);


			colony<small_struct> ss_colony;
			ss_colony.reserve(512);

			for (unsigned int temp = 0; temp != 200; ++temp)
			{
				ss_colony.insert(rand() & 65535);
			}

			ss_colony.sort();

			small_struct ss_previous(0);

			for (colony<small_struct>::iterator current = ss_colony.begin(); current != ss_colony.end(); ++current)
			{
				if (ss_previous > *current)
				{
					sorted = false;
					break;
				}

				ss_previous = *current;
			}

			failpass("Less-than sort with allocation optimization 2 test - large type 1", sorted);


			ss_colony.clear();
			ss_colony.reshape(plf::limits(100, 100));
			ss_colony.reserve(200);

			for (unsigned int temp = 0; temp != 75; ++temp)
			{
				ss_colony.insert(rand() & 65535);
			}

			ss_colony.sort();

			ss_previous = 0;

			for (colony<small_struct>::iterator current = ss_colony.begin(); current != ss_colony.end(); ++current)
			{
				if (ss_previous > *current)
				{
					sorted = false;
					break;
				}

				ss_previous = *current;
			}

			failpass("Less-than sort with allocation optimization 3 test - large type 2", sorted);
		}

		{
			colony<sort_counted> sc_colony;

			for (int counter = 0; counter != 100; ++counter)
			{
				sc_colony.insert(sort_counted(100 - counter));
			}

			sc_colony.sort();

			failpass("Sort destroys its temporary copies test", sort_live_count == 100);
		}

		#ifdef PLF_MOVE_SEMANTICS_SUPPORT
		{
			// A move-only element type takes the copy-to-an-array path, which must not need a copy constructor or copy assignment - in C++11/14 as well, where if constexpr is not available:
			colony<std::unique_ptr<int> > up_colony;

			for (int counter = 0; counter != 100; ++counter)
			{
				up_colony.insert(std::unique_ptr<int>(new int((counter * 37) % 100)));
			}

			up_colony.sort(unique_ptr_pointee_less());

			bool up_sorted = true;
			int up_previous = -1;

			for (colony<std::unique_ptr<int> >::iterator current = up_colony.begin(); current != up_colony.end(); ++current)
			{
				if (*current == NULL || **current < up_previous)
				{
					up_sorted = false;
					break;
				}

				up_previous = **current;
			}

			failpass("Sort move-only type test", up_sorted && up_colony.size() == 100);
		}
		#endif

		{
			title2("Sort with erasures tests");

			// The copy-to-an-array path (int, a type whose copy can throw, a non-trivially-copyable type) and the pointer-tuple path (std::string), with blocks without erasures, few erasures (word scan) and many erasures:
			bool int_sorts = true, greater_sorts = true, custom_sorts = true, throwing_sorts = true, non_trivial_sorts = true, string_sorts = true, string_custom_sorts = true;

			for (int pattern = 0; pattern != 4; ++pattern)
			{
				colony<int> i_colony(plf::limits(50, 100)), g_colony(plf::limits(50, 100)), c_colony;
				colony<sort_throwing_copy> t_colony;
				colony<sort_non_trivial> n_colony(plf::limits(64, 64));
				colony<std::string> s_colony(plf::limits(50, 100)), sc_colony;

				for (int counter = 0; counter != 2000; ++counter)
				{
					const int value = rand() % 1000;
					char text[16];
					sprintf(text, "%d", value);

					i_colony.insert(value);
					g_colony.insert(value);
					c_colony.insert(value);
					t_colony.insert(sort_throwing_copy(value));
					n_colony.insert(sort_non_trivial(value));
					s_colony.insert(std::string(text));
					sc_colony.insert(std::string(text));
				}

				colony<int>::iterator g_current = g_colony.begin(), c_current = c_colony.begin();
				colony<sort_throwing_copy>::iterator t_current = t_colony.begin();
				colony<sort_non_trivial>::iterator n_current = n_colony.begin();
				colony<std::string>::iterator s_current = s_colony.begin(), sc_current = sc_colony.begin();

				for (colony<int>::iterator current = i_colony.begin(); current != i_colony.end();) // Pattern 0 no erasures, 1 one in 64, 2 one in 4, 3 sparse
				{
					const bool erase = (pattern == 1) ? (rand() & 63) == 0 : (pattern == 2) ? (rand() & 3) == 0 : (pattern == 3) ? (rand() & 7) != 0 : false;

					if (erase)
					{
						current = i_colony.erase(current);
						g_current = g_colony.erase(g_current);
						c_current = c_colony.erase(c_current);
						t_current = t_colony.erase(t_current);
						n_current = n_colony.erase(n_current);
						s_current = s_colony.erase(s_current);
						sc_current = sc_colony.erase(sc_current);
					}
					else
					{
						++current;
						++g_current;
						++c_current;
						++t_current;
						++n_current;
						++s_current;
						++sc_current;
					}
				}

				int_sorts = int_sorts && sort_matches_sorted_copy(i_colony, std::less<int>());
				greater_sorts = greater_sorts && sort_matches_sorted_copy(g_colony, std::greater<int>());
				custom_sorts = custom_sorts && sort_matches_sorted_copy(c_colony, sort_by_last_digit());
				throwing_sorts = throwing_sorts && sort_matches_sorted_copy(t_colony, std::less<sort_throwing_copy>());
				non_trivial_sorts = non_trivial_sorts && sort_matches_sorted_copy(n_colony, std::less<sort_non_trivial>());
				string_sorts = string_sorts && sort_matches_sorted_copy(s_colony, std::less<std::string>());
				string_custom_sorts = string_custom_sorts && sort_matches_sorted_copy(sc_colony, sort_by_last_digit());
			}

			failpass("Sort with erasures test", int_sorts);
			failpass("Sort with erasures, greater-than test", greater_sorts);
			failpass("Sort with erasures, custom comparator test", custom_sorts);
			failpass("Sort with erasures, copy that may throw test", throwing_sorts);
			failpass("Sort with erasures, non-trivially-copyable type test", non_trivial_sorts);
			failpass("Sort with erasures, pointer sort test", string_sorts);
			failpass("Sort with erasures, pointer sort custom comparator test", string_custom_sorts);
		}

		{
			// A copy throwing part way through copying the elements into the sort array must leave the colony as it was, and destroy nothing twice. Copies already made into the sort array may not be destroyed, so the live count may be higher than the number of elements:
			bool unchanged = true, no_double_destroy = true, sorts_after = true;

			for (int pattern = 0; pattern != 3; ++pattern) // 0 no erasures, 1 one in 4 erased, 2 sparse
			{
				colony<sort_throws_on_copy> st_colony;

				for (int counter = 0; counter != 240; ++counter) // Few enough for an unused block to hold the sort array (blocks of this type hold at most 255 elements)
				{
					st_colony.insert(sort_throws_on_copy((counter == 120) ? 500 : 501 + rand() % 499)); // Exactly one 500, in the middle
				}

				for (colony<sort_throws_on_copy>::iterator current = st_colony.begin(); current != st_colony.end();)
				{
					const bool erase = (current->number != 500) && ((pattern == 1) ? (rand() & 3) == 0 : (pattern == 2) ? (rand() & 7) != 0 : false);
					current = erase ? st_colony.erase(current) : ++current;
				}

				st_colony.reserve(st_colony.capacity() + 255); // An unused block for the sort array, so the sort array, which is not deallocated when a copy throws, is not a memory leak
				std::vector<sort_throws_on_copy> before(st_colony.begin(), st_colony.end());
				const int live_before = sort_throwing_live_count;
				bool threw = false;
				sort_throwing_armed = true;

				try
				{
					st_colony.sort();
				}
				catch (...)
				{
					threw = true;
				}

				sort_throwing_armed = false;

				unchanged = unchanged && threw && st_colony.size() == before.size() && std::equal(st_colony.begin(), st_colony.end(), before.begin());
				no_double_destroy = no_double_destroy && sort_throwing_live_count >= live_before && sort_throwing_live_count < live_before + static_cast<int>(before.size());

				sorts_after = sorts_after && sort_matches_sorted_copy(st_colony, std::less<sort_throws_on_copy>());
			}

			failpass("Sort with erasures, copy throws test", unchanged);
			failpass("Sort with erasures, copy throws, no double destruction test", no_double_destroy);
			failpass("Sort with erasures, sort after a copy throws test", sorts_after);
		}




		{
			title2("Different insertion-style tests");

			#ifdef PLF_INITIALIZER_LIST_SUPPORT
				colony<int> i_colony({1, 2, 3});

				failpass("Initializer-list constructor test", i_colony.size() == 3);
			#else
				colony<int> i_colony(3, 1);
			#endif

			colony<int> i_colony2(i_colony.begin(), i_colony.end());

			failpass("Range constructor test", i_colony2.size() == 3);

			#ifdef PLF_CPP20_SUPPORT
				std::ranges::take_view<std::ranges::ref_view<plf::colony<int>>> rng = i_colony2 | std::ranges::views::take(2);

				colony<int> i_colony_range(plf::from_range, rng);

				failpass("Rangesv3 constructor test", i_colony_range.size() == 2);
			#endif

			colony<int> i_colony3(5000, 2, plf::limits(100, 250));

			failpass("Fill construction test", i_colony3.size() == 5000);

			{
				// A string literal value is an array, so it must not be taken for the iterator type of a range overload:
				colony<std::string> s_colony(3, "a");
				colony<std::string> s_colony2(3, "b", plf::limits(3, 3));
				colony<const char *> p_colony(4, "c");
				const char *input[2] = {"d", "e"};
				colony<std::string> r_colony(input, input + 2);

				failpass("Fill construction from string literal test", s_colony.size() == 3 && std::count(s_colony.begin(), s_colony.end(), "a") == 3 && s_colony2.size() == 3 && p_colony.size() == 4 && r_colony.size() == 2 && *r_colony.begin() == "d");

				s_colony.assign(4, "f");
				failpass("Fill assign from string literal test", s_colony.size() == 4 && std::count(s_colony.begin(), s_colony.end(), "f") == 4);
			}

			i_colony2.insert(500000, 5);

			failpass("Fill insertion test", i_colony2.size() == 500003);

			{
				colony<int> i_colony4;
				i_colony4.insert(1);
				i_colony4.insert(4, 2);

				failpass("Fill insertion into partly-filled back block test", distance(i_colony4.begin(), i_colony4.end()) == 5);
			}

			{
				colony<unsigned char> uc_colony(5, 1);
				uc_colony.insert(5, 2);
				uc_colony.erase(uc_colony.begin());
				uc_colony.erase(uc_colony.begin());
				uc_colony.insert(2, 3);

				int uc_total = 0;

				for (colony<unsigned char>::iterator it = uc_colony.begin(); it != uc_colony.end(); ++it)
				{
					uc_total += *it;
				}

				failpass("Fill insertion for 1-byte types test", uc_colony.size() == 10 && uc_total == 19);
			}


			std::vector<int> some_ints(500, 2);

			i_colony2.insert(some_ints.begin(), some_ints.end());

			failpass("Range insertion test", i_colony2.size() == 500503);

			#ifdef PLF_CPP20_SUPPORT
				i_colony2.insert(some_ints.begin(), some_ints.cend());

				failpass("Range insertion with differing iterators test", i_colony2.size() == 501003);
			#else
				i_colony2.insert(some_ints.begin(), some_ints.end());
			#endif

			#ifdef PLF_MOVE_SEMANTICS_SUPPORT
	 			i_colony3.insert(plf::make_move_iterator(i_colony2.begin()), plf::make_move_iterator(i_colony2.end()));

	 			failpass("Range move-insertion test", i_colony3.size() == 506003);

				i_colony3.clear();
			#endif

			i_colony2.reset();
			i_colony2.reserve(50000);
			i_colony2.insert(60000, 1);

			int total = 0;

			for (colony<int>::iterator it = i_colony2.begin(); it != i_colony2.end(); ++it)
			{
				total += *it;
			}

			failpass("Reserve + fill insert test", i_colony2.size() == 60000 && total == 60000);


			i_colony2.clear();
			i_colony2.reserve(5000);
			i_colony2.insert(60, 1);

			total = 0;

			for (colony<int>::iterator it = i_colony2.begin(); it != i_colony2.end(); ++it)
			{
				total += *it;
			}

			failpass("Reserve + fill insert test 2", i_colony2.size() == 60 && total == 60);

			i_colony2.insert(6000, 1);

			total = 0;

			for (colony<int>::iterator it = i_colony2.begin(); it != i_colony2.end(); ++it)
			{
				total += *it;
			}

			failpass("Reserve + fill + fill test", i_colony2.size() == 6060 && total == 6060);

			i_colony2.reserve(18000);
			i_colony2.insert(6000, 1);

			total = 0;

			for (colony<int>::iterator it = i_colony2.begin(); it != i_colony2.end(); ++it)
			{
				total += *it;
			}

			failpass("Reserve + fill + fill + reserve + fill test", i_colony2.size() == 12060 && total == 12060);

			i_colony2.clear();
			i_colony2.insert(6000, 2);

			failpass("Clear + fill test", i_colony2.size() == 6000 && *(i_colony2.begin()) == 2);

			i_colony.insert(i_colony2.begin(), i_colony2.end());

			failpass("Range insert when not empty test", i_colony.size() == 6003);

			i_colony2.reset();
			i_colony2.reshape(plf::limits(50, 70));
			i_colony2.reserve(150);

			failpass("Improved reserve accuracy test", i_colony2.capacity() == 150);
		}


		{
			title2("Assign tests");

			colony<int> i_colony(50, 2);

			i_colony.assign(50, 1);

			int total = 0;

			for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
			{
				total += *it;
			}

			failpass("Equal capacity assign test", i_colony.size() == 50 && total == 50);


			i_colony.assign(10, 2);

			total = 0;

			for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
			{
				total += *it;
			}

			failpass("Lesser capacity assign test", i_colony.size() == 10 && total == 20);


			i_colony.assign(2000, 20);

			total = 0;

			for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
			{
				total += *it;
			}

			failpass("Greater capacity assign test", i_colony.size() == 2000 && total == 40000);

			i_colony.clear();


			for (unsigned int internal_loop_counter = 0; internal_loop_counter != 10; ++internal_loop_counter)
			{
				const unsigned int capacity = rand() & 65535;
				i_colony.assign(capacity, 1);

				total = 0;

				for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
				{
					total += *it;
				}

				if (i_colony.size() != capacity)
				{
					printf("Fuzz-test assign capacity Fail: global loop counter: %u, internal loop counter: %u.\n", looper, internal_loop_counter);
					getchar();
					abort();
				}

				if (i_colony.size() != static_cast<unsigned int>(total))
				{
					printf("Fuzz-test assign sum Fail: global loop counter: %u, internal loop counter: %u.\n", looper, internal_loop_counter);
					getchar();
					abort();
				}
			}

			message("Fuzz-test assign passed.");


			i_colony.clear();

			std::vector<int> i_vector;

			for (int counter = 1; counter != 11; ++counter)
			{
				i_vector.push_back(counter);
			}

			i_colony.assign(i_vector.begin(), i_vector.end());

			colony<int>::iterator it = i_colony.begin();
			bool fail = false;

			for (int counter = 1; counter != 11; ++counter, ++it)
			{
				if (*it != counter)
				{
					fail = true;
					break;
				}
			}

			failpass("Range assign test", i_colony.size() == 10 && !fail);


			i_colony.clear();


			for (unsigned int internal_loop_counter = 0; internal_loop_counter != 10; ++internal_loop_counter)
			{
				const unsigned int capacity = rand() & 65535;
				i_vector.assign(capacity, 1);
				i_colony.assign(i_vector.begin(), i_vector.end());

				total = 0;

				for (colony<int>::iterator it3 = i_colony.begin(); it3 != i_colony.end(); ++it3)
				{
					total += *it3;
				}

				if (i_colony.size() != capacity)
				{
					printf("Fuzz-test range assign capacity Fail: global loop counter: %u, internal loop counter: %u.\n", looper, internal_loop_counter);
					getchar();
					abort();
				}

				if (i_colony.size() != static_cast<unsigned int>(total))
				{
					printf("Fuzz-test range assign sum Fail: global loop counter: %u, internal loop counter: %u.\n", looper, internal_loop_counter);
					getchar();
					abort();
				}
			}

			message("Fuzz-test range assign passed.");


			i_colony.clear();


			for (unsigned int internal_loop_counter = 0; internal_loop_counter != 10; ++internal_loop_counter)
			{
				const unsigned int capacity = rand() & 65535;
				i_vector.assign(capacity, 1);
				i_colony.assign(i_vector.begin(), i_vector.end());

				total = 0;
				unsigned int subtract = 0;

				for (colony<int>::iterator it3 = i_colony.begin(); it3 != i_colony.end(); )
				{
					if ((rand() & 7) == 0)
					{
						it3 = i_colony.erase(it3);
						++subtract;
					}
					else
					{
						total += *it3++;
					}
				}

				if (i_colony.size() != capacity - subtract)
				{
					printf("Fuzz-test range assign + erase capacity Fail: global loop counter: %u, internal loop counter: %u.\n", looper, internal_loop_counter);
					getchar();
					abort();
				}

				if (i_colony.size() != static_cast<unsigned int>(total))
				{
					printf("Fuzz-test range assign + erase sum Fail: global loop counter: %u, internal loop counter: %u.\n", looper, internal_loop_counter);
					getchar();
					abort();
				}
			}

			message("Fuzz-test range assign + erase passed.");

			{ // Range assign that ends on an erased slot at the end of a full back block, with a non-trivially-destructible type
				colony<small_struct_non_trivial> nt_colony(plf::limits(3, 3));
				nt_colony.insert(1);
				nt_colony.insert(2);
				nt_colony.erase(nt_colony.insert(3)); // 1 2 _ : the block is full and ends with an erased slot

				const std::vector<small_struct_non_trivial> nt_vector(3, small_struct_non_trivial(4));
				nt_colony.assign(nt_vector.begin(), nt_vector.end());

				failpass("Range assign to end of full back block test", nt_colony.size() == 3 && nt_colony.begin()->number == 4);
			}


			{ // Range assign from another colony's iterators, const iterators and reverse iterators, with erased elements in the source
				colony<int> source;

				for (int counter = 0; counter != 1000; ++counter)
				{
					source.insert(counter);
				}

				for (colony<int>::iterator current = source.begin(); current != source.end();)
				{
					if (*current % 3 == 0)
					{
						current = source.erase(current);
					}
					else
					{
						++current;
					}
				}

				colony<int> forward_colony, const_colony, reverse_colony;
				forward_colony.assign(source.begin(), source.end());
				const_colony.assign(source.cbegin(), source.cend());
				reverse_colony.assign(source.rbegin(), source.rend());

				bool forward_matches = forward_colony.size() == source.size(), const_matches = const_colony.size() == source.size(), reverse_matches = reverse_colony.size() == source.size();

				colony<int>::iterator forward_it = forward_colony.begin(), const_it = const_colony.begin(), reverse_it = reverse_colony.begin();
				colony<int>::reverse_iterator source_reverse_it = source.rbegin();

				for (colony<int>::iterator current = source.begin(); current != source.end() && forward_matches && const_matches && reverse_matches; ++current, ++forward_it, ++const_it, ++reverse_it, ++source_reverse_it)
				{
					forward_matches = *forward_it == *current;
					const_matches = *const_it == *current;
					reverse_matches = *reverse_it == *source_reverse_it;
				}

				failpass("Range assign from colony iterators test", forward_matches);
				failpass("Range assign from colony const_iterators test", const_matches);
				failpass("Range assign from colony reverse_iterators test", reverse_matches);
			}


			i_colony.clear();

			#ifdef PLF_INITIALIZER_LIST_SUPPORT
				i_colony.assign({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
				it = i_colony.begin();

				for (int counter = 1; counter != 11; ++counter, ++it)
				{
					if (*it != counter)
					{
						fail = true;
						break;
					}
				}

				failpass("Initializer_list assign test", i_colony.size() == 10 && !fail);
			#endif
		}


		#ifdef PLF_VARIADICS_SUPPORT
			{
				title2("Perfect Forwarding tests");

				colony<perfect_forwarding_test> pf_colony;

				int lvalue = 0;
				int &lvalueref = lvalue;

				pf_colony.emplace(7, lvalueref);

				failpass("Perfect forwarding test", (*pf_colony.begin()).success);
				failpass("Perfect forwarding test 2", lvalueref == 1);
			}


			{
				title2("Basic emplace test");

				colony<small_struct> ss_colony;
				int total1 = 0, total2 = 0;

				for (int counter = 0; counter != 100; ++counter)
				{
					ss_colony.emplace(counter);
					total1 += counter;
				}

				for (colony<small_struct>::iterator it = ss_colony.begin(); it != ss_colony.end(); ++it)
				{
					total2 += it->number;
				}

				failpass("Basic emplace test", total1 == total2);
				failpass("Basic emplace test 2", ss_colony.size() == 100);
			}


			{
				title2("Non-copyable type test");

				colony<non_copyable_type> temp;

				temp.emplace(1);
				temp.emplace(2);

				failpass("Non-copyable size test", temp.size() == 2);
			}

		#endif


		{
			title2("Misc function tests");

			colony<int> colony1;
			colony1.reshape(plf::limits(50, 100));

			colony1.insert(27);

			failpass("Change_group_sizes min-size test", colony1.capacity() == 50);

			for (int counter = 0; counter != 100; ++counter)
			{
				colony1.insert(counter);
			}

			failpass("Change_group_sizes max-size test", colony1.capacity() == 200);

			colony1.clear();
			colony1.reshape(plf::limits(200, 255));

			colony1.insert(27);

			failpass("Reshape min-size test", colony1.capacity() == 200);

			plf::limits temp_limits = colony1.block_capacity_limits();

			failpass("get_block_capacity_limits test", temp_limits.min == 200 && temp_limits.max == 255);

			temp_limits = plf::colony<int>::block_capacity_hard_limits();

			failpass("get_block_capacity_limits test", temp_limits.min == 3 && temp_limits.max == 255);

			for (int counter = 0; counter != 3300; ++counter)
			{
				colony1.insert(counter);
			}

			failpass("Reshape max-size test", colony1.capacity() == 3460);

			colony1.reshape(plf::limits(150, 150));

			failpass("Reshape test 3", colony1.capacity() == 3450);

			colony1.reshape(plf::limits(200, 200));

			failpass("Reshape test 4", colony1.capacity() == 3400);

			{ // reshape() on an empty colony, where the first block no longer fits and is deallocated but a later block is kept
				colony<int> reshaped;

				for (int counter = 0; counter != 1000; ++counter)
				{
					reshaped.insert(counter);
				}

				reshaped.clear();
				reshaped.reshape(plf::limits(100, 255));

				for (int counter = 0; counter != 300; ++counter)
				{
					reshaped.insert(counter);
				}

				int total = 0;

				for (colony<int>::iterator current = reshaped.begin(); current != reshaped.end(); ++current)
				{
					total += *current;
				}

				failpass("Reshape after clear test", reshaped.size() == 300 && total == 44850);
			}

			failpass("max_elements_per_allocation test", colony<int>::max_elements_per_allocation(0) == 0 && colony<int>::max_elements_per_allocation(64) == 12 && max_elements_per_allocation_correct<colony<char> >() && max_elements_per_allocation_correct<colony<int> >() && max_elements_per_allocation_correct<colony<double> >() && max_elements_per_allocation_correct<colony<small_struct> >() && max_elements_per_allocation_correct<colony<int, std::allocator<int>, plf::memory_use> >());

			#ifdef PLF_CPP20_SUPPORT
				static_assert(colony<int>::max_elements_per_allocation(64) == 12);
			#endif

		}

		{
			title2("Splice tests");

			{
				colony<int> colony1, colony2;

				for (int number = 0; number != 20; ++number)
				{
					colony1.insert(number);
					colony2.insert(number + 20);
				}

				colony1.splice(colony2);

				int check_number = 0;
				bool fail = false;

				for (colony<int>::iterator current = colony1.begin(); current != colony1.end(); ++current)
				{
					if (check_number++ != *current)
					{
						fail = true;
					}
				}

				failpass("Small splice test 1", fail == false);
			}


			{
				colony<int> colony1, colony2;

				for(int number = 0; number != 100; ++number)
				{
					colony1.insert(number);
					colony2.insert(number + 100);
				}

				colony1.splice(colony2);

				int check_number = 0;
				bool fail = false;

				for (colony<int>::iterator current = colony1.begin(); current != colony1.end(); ++current)
				{
					if (check_number++ != *current)
					{
						fail = true;
					}
				}

				failpass("Small splice test 2", fail == false);
			}


			{
				colony<int> colony1, colony2;

				for(int number = 0; number != 100000; ++number)
				{
					colony1.insert(number);
					colony2.insert(number + 100000);
				}

				colony1.splice(colony2);

				int check_number = 0;
				bool fail = false;

				for (colony<int>::iterator current = colony1.begin(); current != colony1.end(); ++current)
				{
					if (check_number++ != *current)
					{
						fail = true;
					}
				}

				failpass("Large splice test 1", fail == false);
			}


			{
				colony<int> colony1, colony2;

				for(int number = 0; number != 100; ++number)
				{
					colony1.insert(number);
					colony2.insert(number + 100);
				}


				for (colony<int>::iterator current = colony2.begin(); current != colony2.end();)
				{
					if ((rand() & 7) == 0)
					{
						current = colony2.erase(current);
					}
					else
					{
						++current;
					}
				}


				colony1.splice(colony2);

				int check_number = -1;
				bool fail = false;

				for (colony<int>::iterator current = colony1.begin(); current != colony1.end(); ++current)
				{
					if (check_number >= *current)
					{
						fail = true;
					}

					check_number = *current;
				}

				failpass("Erase + splice test 1", fail == false);
			}



			{
				colony<int> colony1, colony2;

				for(int number = 0; number != 100; ++number)
				{
					colony1.insert(number);
					colony2.insert(number + 100);
				}



				for (colony<int>::iterator current = colony2.begin(); current != colony2.end();)
				{
					if ((rand() & 3) == 0)
					{
						current = colony2.erase(current);
					}
					else
					{
						++current;
					}
				}


				for (colony<int>::iterator current = colony1.begin(); current != colony1.end();)
				{
					if ((rand() & 1) == 0)
					{
						current = colony1.erase(current);
					}
					else
					{
						++current;
					}
				}


				colony1.splice(colony2);

				int check_number = -1;
				bool fail = false;

				for (colony<int>::iterator current = colony1.begin(); current != colony1.end(); ++current)
				{
					if (check_number >= *current)
					{
						fail = true;
					}

					check_number = *current;
				}

				failpass("Erase + splice test 2", fail == false);
			}



			{
				colony<int> colony1, colony2;

				colony1.reshape(plf::limits(200, 200));
				colony2.reshape(plf::limits(200, 200));

				for(int number = 0; number != 100; ++number)
				{
					colony1.insert(number + 150);
				}


				for(int number = 0; number != 150; ++number)
				{
					colony2.insert(number);
				}


				colony1.splice(colony2);

				int check_number = -1;
				bool fail = false;

				for (colony<int>::iterator current = colony1.begin(); current != colony1.end(); ++current)
				{
					if (check_number >= *current)
					{
						fail = true;
					}

					check_number = *current;
				}

				failpass("Unequal size splice test 1", fail == false);
			}



			{
				colony<int> colony1(plf::limits(200, 200)), colony2(plf::limits(200, 200));

				for(int number = 0; number != 100; ++number)
				{
					colony1.insert(100 - number);
				}


				for(int number = 0; number != 150; ++number)
				{
					colony2.insert(250 - number);
				}


				colony1.splice(colony2);

				int check_number = 255;
				bool fail = false;

				for (colony<int>::iterator current = colony1.begin(); current != colony1.end(); ++current)
				{
					if (check_number < *current)
					{
						fail = true;
					}

					check_number = *current;
				}

				failpass("Unequal size splice test 2", fail == false);
			}



			{
				colony<int> colony1, colony2;

				for(int number = 0; number != 100000; ++number)
				{
					colony1.insert(number + 200000);
				}


				for(int number = 0; number != 200000; ++number)
				{
					colony2.insert(number);
				}

				for (colony<int>::iterator current = colony2.begin(); current != colony2.end();)
				{
					if ((rand() & 1) == 0)
					{
						current = colony2.erase(current);
					}
					else
					{
						++current;
					}
				}


				for (colony<int>::iterator current = colony1.begin(); current != colony1.end();)
				{
					if ((rand() & 1) == 0)
					{
						current = colony1.erase(current);
					}
					else
					{
						++current;
					}
				}


				colony1.erase(--(colony1.end()));
				colony2.erase(--(colony2.end()));

				const unsigned int total = static_cast<unsigned int>(colony1.size() + colony2.size());

				colony1.splice(colony2);

				unsigned int check_number = 0;

				for (colony<int>::iterator current = colony1.begin(); current != colony1.end(); ++current)
				{
					++check_number;
				}

				failpass("Large unequal size + erase splice test 1", check_number == total);


				do
				{
					for (colony<int>::iterator current = colony1.begin(); current != colony1.end();)
					{
						if ((rand() & 3) == 0)
						{
							current = colony1.erase(current);
						}
						else if ((rand() & 7) == 0)
						{
							colony1.insert(433);
							++current;
						}
						else
						{
							++current;
						}
					}

				} while (!colony1.empty());

				failpass("Post-splice insert-and-erase randomly till-empty test", colony1.size() == 0);
			}
		}

		{
			title2("erase_if tests");

			colony<int> i_colony(100, 100);

			i_colony.insert(100, 200);
			colony<int> i_colony2 = i_colony;

			std::erase(i_colony, 100);

			failpass("non-member erase test 1", i_colony.size() == 100);

			erase(i_colony2, 200);


			failpass("non-member erase test 2", i_colony2.size() == 100);


			i_colony.clear();

			for(int count = 0; count != 1000; ++count)
			{
				i_colony.insert((rand() & 1));
			}

			i_colony2 = i_colony;

			const int count0 = static_cast<int>(std::count(i_colony.begin(), i_colony.end(), 0));
			const int count1 = 1000 - count0;

			erase(i_colony, 0);
			failpass("random non-member erase test 1", static_cast<int>(i_colony.size()) == count1);

			erase(i_colony2, 1);
			failpass("random non-member erase test 2",  static_cast<int>(i_colony2.size()) == count0);


			i_colony.clear();

			for(int count = 0; count != 1000; ++count)
			{
				i_colony.insert(count);
			}

			#ifdef PLF_MOVE_SEMANTICS_SUPPORT // approximating checking for C++11 here
				erase_if(i_colony, std::bind(std::greater<int>(), std::placeholders::_1, 499));
			#else // C++03 or lower
				erase_if(i_colony, std::bind2nd(std::greater<int>(), 499));
			#endif

			failpass("erase_if test",	static_cast<int>(i_colony.size()) == 500);

			bool erase_if_matches = true, predicate_called_once = true;
			erase_if_pattern_test<int>(erase_if_matches, predicate_called_once);
			failpass("erase_if pattern test", erase_if_matches);
			failpass("erase_if predicate call count test", predicate_called_once);

			erase_if_matches = predicate_called_once = true;
			erase_if_pattern_test<std::string>(erase_if_matches, predicate_called_once); // Non-trivial element type, which also has a 16-bit skipfield
			failpass("erase_if non-trivial type pattern test", erase_if_matches);
			failpass("erase_if non-trivial type predicate call count test", predicate_called_once);

			{
				colony<int> empty_colony;
				std::size_t calls = 0;
				const erase_if_counting_predicate predicate = {&calls, 2, 1};
				failpass("erase_if empty colony test", erase_if(empty_colony, predicate) == 0 && calls == 0 && empty_colony.empty());
			}

			{
				bool stateful_matches = true;

				for (int prior_erasure = 0; prior_erasure != 3; ++prior_erasure)
				{
					const int nths[] = {37, 100, 1000};

					for (int index = 0; index != 3; ++index)
					{
						if (!erase_if_stateful_test<int>(prior_erasure, nths[index]) || !erase_if_stateful_test<std::string>(prior_erasure, nths[index]))
						{
							stateful_matches = false;
						}
					}
				}

				failpass("erase_if stateful predicate test", stateful_matches);
			}

		}

		{
			title2("Element destruction tests");
			destruction_tests<1>();
			destruction_tests<24>();
			empty_back_block_tests<1>();
			empty_back_block_tests<24>();
		}

		{
			title2("Copying from a colony tests");

			copy_test_results int_results, struct_results, string_results, throwing_type_results;
			bool exceptions_handled = true;

			for (int pattern = 0; pattern != 6; ++pattern)
			{
				for (int limits = 0; limits != 2; ++limits)
				{
					copy_from_colony_test<int>(pattern, limits, 10, int_results);
					copy_from_colony_test<small_struct>(pattern, limits, 4, struct_results);
					copy_from_colony_test<std::string>(pattern, limits, 2, string_results);
					copy_from_colony_test<copy_throws>(pattern, limits, 2, throwing_type_results);
					exceptions_handled = exceptions_handled && copy_from_colony_throw_test(pattern, limits);
				}
			}

			failpass("Copy constructor from colony test", int_results.copies && struct_results.copies && string_results.copies && throwing_type_results.copies);
			failpass("Allocator-extended copy constructor from colony test", int_results.allocator_copies && struct_results.allocator_copies && string_results.allocator_copies && throwing_type_results.allocator_copies);
			failpass("Range insert from colony iterators test", int_results.inserts && struct_results.inserts && string_results.inserts && throwing_type_results.inserts);
			failpass("Range construction from colony iterators test", int_results.range_constructions && struct_results.range_constructions && string_results.range_constructions && throwing_type_results.range_constructions);
			failpass("Assign from colony iterators test", int_results.assigns && struct_results.assigns && string_results.assigns && throwing_type_results.assigns);
			failpass("Assign from colony iterators to empty colony test", int_results.empty_assigns && struct_results.empty_assigns && string_results.empty_assigns && throwing_type_results.empty_assigns);
			failpass("Copying from a colony exceptions test", exceptions_handled && copy_throws_live_count == 0);
			failpass("Copying from a colony block boundary test", block_boundary_fill_test(0) && block_boundary_fill_test(1) && block_boundary_fill_test(2));

			{
				const colony<int> empty_source;
				colony<int> erased_source(500, 1);
				erased_source.erase(erased_source.begin(), erased_source.end());
				const colony<int> empty_copy(empty_source), erased_copy(erased_source), allocator_empty_copy(empty_source, empty_source.get_allocator());
				colony<int> destination(10, 2);
				destination.insert(empty_source.begin(), empty_source.end());
				destination.insert(erased_source.begin(), erased_source.end());
				failpass("Copying from an empty colony test", empty_copy.empty() && erased_copy.empty() && allocator_empty_copy.empty() && empty_copy.begin() == empty_copy.end() && destination.size() == 10);
			}
		}

		#ifdef PLF_CPP20_SUPPORT
		{
			title2("range/fill-insertion when skipblocks are present testing");

			colony<int> i_colony(100, 1);

			i_colony.erase(next(i_colony.begin(), 80), i_colony.end());
			i_colony.insert(10, 2);

			unsigned int total1s = 0, total2s = 0, total3s = 0;

			for (colony<int>::const_iterator current = i_colony.begin(); current != i_colony.end(); ++current)
			{
				total1s += *current == 1;
				total2s += *current == 2;
			}

			failpass("fill-insert smaller than pre-existing skipblock test", static_cast<int>(i_colony.size()) == 90 && total1s == 80 && total2s == 10);

			i_colony.insert(40, 3);
			total2s = 0;

			for (colony<int>::const_iterator current = i_colony.begin(); current != i_colony.end(); ++current)
			{
				total2s += *current == 2;
				total3s += *current == 3;
			}

			failpass("fill-insert larger than pre-existing skipblock test", static_cast<int>(i_colony.size()) == 130 && total2s == 10 && total3s == 40);



			i_colony.erase(next(i_colony.begin(), 120), i_colony.end());
			i_colony.insert({4, 4, 4, 4, 4});

			unsigned int total4s = 0;
			total3s = 0;

			for (colony<int>::const_iterator current = i_colony.begin(); current != i_colony.end(); ++current)
			{
				total3s += *current == 3;
				total4s += *current == 4;
			}

			failpass("range-insert smaller than pre-existing skipblock test", static_cast<int>(i_colony.size()) == 125 && total3s == 30 && total4s == 5);

			i_colony.insert({5, 5, 5, 5, 5, 5, 5, 5, 5, 5});

			total4s = 0;
			unsigned int total5s = 0;

			for (colony<int>::const_iterator current = i_colony.begin(); current != i_colony.end(); ++current)
			{
				total4s += *current == 4;
				total5s += *current == 5;
			}

			failpass("range-insert larger than pre-existing skipblock test", static_cast<int>(i_colony.size()) == 135 && total4s == 5 && total5s == 10);

		}
		#endif

		{
			title2("data() tests");

			colony<int> i_colony(10000, 5);

			int sum1 = 0, sum2 = 0, range1 = 0, range2 = 0;


			// Erase half of all elements and sum the rest:
			for (colony<int>::iterator it = i_colony.begin(); it != i_colony.end(); ++it)
			{
				it = i_colony.erase(it);
				sum1 += *it;
				++range1;
			}


			colony<int>::colony_data *data = i_colony.data();

			// Manually sum using raw memory blocks:
			for (unsigned int block_num = 0; block_num != data->number_of_blocks; ++block_num)
			{
				colony<int>::aligned_element_struct *current_element = data->block_pointers[block_num];
				const unsigned char *bitfield_location = data->bitfield_pointers[block_num];
				const size_t capacity = data->block_capacities[block_num];

				size_t char_index = 0;
				unsigned char offset = 0;

				for (size_t index = 0; index != capacity; ++index, ++current_element)
				{
					if ((bitfield_location[char_index] >> offset) & 1)
					{
						sum2 += *(reinterpret_cast<int *>(current_element));
						++range2;
					}

					if (++offset == 8)
					{
						offset = 0;
						++char_index;
					}
				}
			}

			delete data;

			failpass("Manual summing pass over elements obtained from data()", (sum1 == sum2) && (range1 == range2));
		}


		{
			title2("range/fill insert partial recovery tests");

			colony<exceptions_test> i_colony;
			exceptions_test input_data[10] = {6, 6, 6, 6, 4, 6, 6, 6, 6, 1};

			try
			{
				i_colony.insert(input_data, input_data + 10);
			}
			catch(...)
			{} // do nothing

			int accumulator = 0;
			for (colony<exceptions_test>::iterator current = i_colony.begin(); current != i_colony.end(); ++current)
			{
				accumulator += current->num;
			}

			failpass("fill-insert initializer-list exceptions test", static_cast<int>(i_colony.size()) == 4 && accumulator == 24);

			i_colony.clear();
			i_colony.reserve(20);
			i_colony.insert(20, input_data[9]); // 20 1's
			colony<exceptions_test>::iterator start = next(i_colony.begin(), 5), end = next(i_colony.begin(), 15);
			i_colony.erase(start, end); // Create skip-block in middle of colony sequence

			try
			{
				i_colony.insert(input_data, input_data + 10); // Insert directly into the skipblock and trigger an exception
			}
			catch(...)
			{} // do nothing

			accumulator = 0;
			for (colony<exceptions_test>::iterator current = i_colony.begin(); current != i_colony.end(); ++current)
			{
				accumulator += current->num;
			}

			failpass("fill-insert into skip-block exceptions test", static_cast<int>(i_colony.size()) == 14 && accumulator == 34);
		}
	}


	title1("Test Suite PASS - Press ENTER to Exit");
	getchar();

	return 0;
}
