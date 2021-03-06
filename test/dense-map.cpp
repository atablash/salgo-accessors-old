#include <salgo/dense-map.hpp>

#include <gtest/gtest.h>

#include <chrono>

using namespace salgo;

using namespace std::chrono;



TEST(Deque_dense_map, push_back_and_read) {

	Dense_Map<int>::BUILDER::Deque::BUILD m;
	m[-1] = -1;
	m.push_back(0);
	m.push_back(1);
	m.push_back(2);
	m.push_back(3);
	m.push_back(4);

	EXPECT_EQ(6, m.size());

	EXPECT_EQ(-1, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());

	EXPECT_EQ(-1, m[-1]);
	EXPECT_EQ(0, m[0]);
	EXPECT_EQ(1, m[1]);
	EXPECT_EQ(2, m[2]);
	EXPECT_EQ(3, m[3]);
	EXPECT_EQ(4, m[4]);

	EXPECT_EQ(false, m[5].exists);

	EXPECT_EQ(-1, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());
}





TEST(Deque_dense_map, push_back_and_read_with_offset) {

	Dense_Map<int>::BUILDER::Deque::BUILD m;
	m[-10] = 42;
	m.push_back(1);
	m.push_back(2);
	m.push_back(3);
	m.push_back(4);
	m.push_back(5);

	EXPECT_EQ(6, m.size());

	EXPECT_EQ(-10, m.domain_begin());
	EXPECT_EQ(-10 + 6, m.domain_end());

	EXPECT_EQ(42, m[-10]);
	EXPECT_EQ(1, m[-9]);
	EXPECT_EQ(2, m[-8]);
	EXPECT_EQ(3, m[-7]);
	EXPECT_EQ(4, m[-6]);
	EXPECT_EQ(5, m[-5]);

	EXPECT_EQ(false, m[5].exists);

	EXPECT_EQ(-10, m.domain_begin());
	EXPECT_EQ(-10 + 6, m.domain_end());
}






TEST(Deque_dense_map, test_delete) {

	Dense_Map<int>::BUILDER::Deque::Erasable::BUILD m;
	m.push_back(1);
	m.push_back(2);
	m.push_back(3);
	m.push_back(4);
	m.push_back(5);

	EXPECT_EQ(5, m.size());

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());

	m[0].erase();
	m[2].erase();
	m[4].erase();


	EXPECT_EQ(true, m[1].exists);
	EXPECT_EQ(false, m[2].exists);

	int sum = 0;
	for(auto e : m) sum += e;
	EXPECT_EQ(6, sum);
}




TEST(Deque_dense_map, test_delete_with_offset) {

	Dense_Map<int>::BUILDER::Deque::Erasable::BUILD m;
	m[-10] = 42;
	m.push_back(1);
	m.push_back(2);
	m.push_back(3);
	m.push_back(4);
	m.push_back(5);

	EXPECT_EQ(6, m.size());

	EXPECT_EQ(-10, m.domain_begin());
	EXPECT_EQ(-10 + 6, m.domain_end());

	m[-9].erase();
	m[-7].erase();
	m[-5].erase();

	int sum = 0;
	for(auto e : m) sum += e;
	EXPECT_EQ(42 + 2 + 4, sum);
}






TEST(Deque_dense_map, exists) {

	Dense_Map<int>::BUILDER::Deque::Erasable::BUILD m;
	m.push_back(1);
	m.push_back(2);
	m.push_back(3);
	m.push_back(4);
	m.push_back(5);

	EXPECT_EQ(5, m.size());

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());

	m[0].erase();
	m[2].erase();
	m[4].erase();

	EXPECT_EQ(true, m[3].exists);
	EXPECT_EQ(false, m[4].exists);
	EXPECT_EQ(false, m[5].exists);
	EXPECT_EQ(false, m[100].exists);

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());
}


TEST(Deque_dense_map, auto_resize) {

	Dense_Map<int>::BUILDER::Deque::BUILD m;

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(0, m.domain_end());

	EXPECT_EQ(false, m[100].exists);

	m[100] = 123;
	EXPECT_EQ(123, m[100]);

	EXPECT_EQ(1, m.size());

	EXPECT_EQ(true, m[100].exists);
	EXPECT_EQ(false, m[0].exists);
	EXPECT_EQ(false, m[10].exists);
	EXPECT_EQ(false, m[200].exists);

	EXPECT_EQ(100, m.domain_begin());
	EXPECT_EQ(101, m.domain_end());

	m[150] = 234;
	EXPECT_EQ(123, m[100]);
	EXPECT_EQ(234, m[150]);

	EXPECT_EQ(51, m.size()); // elements in the middle are inserted (not ERASABLE)

	EXPECT_EQ(100, m.domain_begin());
	EXPECT_EQ(151, m.domain_end());
}





TEST(Deque_dense_map, auto_resize_back) {

	Dense_Map<int>::BUILDER::Deque::Erasable::BUILD m;

	m[5] = 5;
	m[-1] = -1;

	EXPECT_EQ(5, m[5]);
	EXPECT_EQ(-1, m[-1]);

	EXPECT_EQ(2, m.size());

	EXPECT_EQ(true, m[5].exists);
	EXPECT_EQ(true, m[-1].exists);
	EXPECT_EQ(false, m[4].exists);

	EXPECT_EQ(-1, m.domain_begin());
	EXPECT_EQ(6, m.domain_end());

	int sum = 0;
	for(auto e : m) sum += e;
	EXPECT_EQ(4, sum);
}





TEST(Deque_dense_map, proxy_operator_brackets) {

	struct S {
		int member;
	};

	Dense_Map<S> m;

	m[5] = S{42};

	EXPECT_EQ(42, m[5].val().member);

	S& ss = m[5];
	EXPECT_EQ(42, ss.member);

	EXPECT_EQ(1, m.size());
}

TEST(Deque_dense_map, iterate_and_erase) {

	Dense_Map<int> m = {-1, 2, -3, 4, -12};

	EXPECT_EQ(5, m.size());

	for(auto e : m) {
		if(e < 0.0) {
			e.erase();
		}
	}

	int sum = 0;
	for(const auto& e : m) {
		sum += e;
	}
	EXPECT_EQ(6, sum);

	EXPECT_EQ(2, m.size());
}

TEST(Deque_dense_map, iterate_and_erase_next) {

	Dense_Map<int> m = {-1, 2, -3, 4, -12, 3, 5, 5};

	EXPECT_EQ(8, m.size());

	for(auto e : m) {
		while(e.next().exists && e >= e.next()) {
			e.next().erase();
		}
	}

	int sum = 0;
	for(const auto& e : m) {
		sum += e;
	}
	EXPECT_EQ(10, sum);

	EXPECT_EQ(4, m.size());
}


TEST(Deque_dense_map, construction_1) {

	Dense_Map<int> m = {5};
	EXPECT_EQ(1, m.size());
	ASSERT_TRUE(m[0].exists);
	EXPECT_EQ(5, m[0]);
}


TEST(Deque_dense_map, construction_2) {
	Dense_Map<int> m(5);
	m.push_back(2);
	EXPECT_EQ(1, m.size());
	ASSERT_TRUE(m[5].exists);
	EXPECT_EQ(2, m[5]);

	EXPECT_FALSE(m[0].exists);
}






TEST(Deque_dense_map, const_test) {

	Dense_Map<int>::BUILDER::Deque::BUILD m;
	m.push_back(1);
	m.push_back(2);
	m.push_back(3);
	m.push_back(4);
	m.push_back(5);

	const auto& c = m;

	EXPECT_EQ(5, m.size());

	EXPECT_EQ(0, c.domain_begin());
	EXPECT_EQ(5, c.domain_end());

	EXPECT_EQ(true, c[2].exists);

	EXPECT_EQ(4, c[3]);
}



TEST(Performance, Deque_dense_map) {
	const int iters = 200;
	const int elements = 100'000;

	double my_time = 0;
	double other_time = 0;

	long long my_result = 0;
	long long other_result = 0;

	for(int iter = 0; iter < iters; ++iter) {

		
		{
			auto t0 = steady_clock::now();


			Dense_Map<int>::BUILDER::Deque::BUILD m;

			for(int i=0; i<elements; ++i) m.push_back(i);
			
			//std::random_shuffle(m.begin(), m.end());

			long long result = 0;
			for(auto e : m) result += e;
			my_result += result;

			duration<double> dur = steady_clock::now() - t0;
			my_time += dur.count();
		}
		
		{
			auto t0 = steady_clock::now();

			struct Node {
				bool exists = false;
				int value;
			};

			std::deque<Node> m;

			for(int i=0; i<elements; ++i) m.push_back({true,i});

			long long result = 0;
			//for(int i=0; i<elements; ++i) result += m[i].value;
			for(auto e : m) result += e.value;
			other_result += result;

			duration<double> dur = steady_clock::now() - t0;
			other_time += dur.count();

		}
	}


	LOG(INFO) << my_time << "s vs " << other_time << "s";


	EXPECT_EQ(my_result, other_result);

	EXPECT_LT(my_time, other_time * 5);
}







































TEST(Dense_map, test_sizeof) {
	EXPECT_LT(
		sizeof(Dense_Map<int>::BUILDER::Vector::BUILD),
		sizeof(Dense_Map<int>::BUILDER::Deque::BUILD));
}





































TEST(Vector_dense_map, push_back_and_read) {

	Dense_Map<int>::BUILDER::Vector::BUILD m;
	m.push_back(0);
	m.push_back(1);
	m.push_back(2);
	m.push_back(3);
	m.push_back(4);

	EXPECT_EQ(5, m.size());

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());

	EXPECT_EQ(0, m[0]);
	EXPECT_EQ(1, m[1]);
	EXPECT_EQ(2, m[2]);
	EXPECT_EQ(3, m[3]);
	EXPECT_EQ(4, m[4]);

	EXPECT_EQ(false, m[5].exists);

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());
}







TEST(Vector_dense_map, test_delete) {

	Dense_Map<int>::BUILDER::Vector::Erasable::BUILD m;
	m.push_back(1);
	m.push_back(2);
	m.push_back(3);
	m.push_back(4);
	m.push_back(5);

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());

	m[0].erase();
	m[2].erase();
	m[4].erase();


	EXPECT_EQ(true, m[1].exists);
	EXPECT_EQ(false, m[2].exists);

	int sum = 0;
	for(auto e : m) sum += e;
	EXPECT_EQ(6, sum);
}




TEST(Vector_dense_map, exists) {

	Dense_Map<int>::BUILDER::Vector::Erasable::BUILD m;
	m.push_back(1);
	m.push_back(2);
	m.push_back(3);
	m.push_back(4);
	m.push_back(5);

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());

	m[0].erase();
	m[2].erase();
	m[4].erase();

	EXPECT_EQ(true, m[3].exists);
	EXPECT_EQ(false, m[4].exists);
	EXPECT_EQ(false, m[5].exists);
	EXPECT_EQ(false, m[100].exists);

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(5, m.domain_end());
}





TEST(Vector_dense_map, auto_resize) {

	Dense_Map<int>::BUILDER::Vector::Erasable::BUILD m;

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(0, m.domain_end());

	EXPECT_EQ(false, m[100].exists);

	m[100] = 123;
	EXPECT_EQ(123, m[100]);

	EXPECT_EQ(true, m[100].exists);
	EXPECT_EQ(false, m[0].exists);
	EXPECT_EQ(false, m[10].exists);
	EXPECT_EQ(false, m[200].exists);

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(101, m.domain_end());

	m[150] = 234;
	EXPECT_EQ(123, m[100]);
	EXPECT_EQ(234, m[150]);

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(151, m.domain_end());
}





TEST(Vector_dense_map, auto_resize_back) {

	Dense_Map<int>::BUILDER::Vector::Erasable::BUILD m;

	m[5] = 5;
	m[1] = 1;

	EXPECT_EQ(5, m[5]);
	EXPECT_EQ(1, m[1]);

	EXPECT_EQ(true, m[5].exists);
	EXPECT_EQ(true, m[1].exists);
	EXPECT_EQ(false, m[4].exists);

	EXPECT_EQ(0, m.domain_begin());
	EXPECT_EQ(6, m.domain_end());

	int sum = 0;
	for(auto e : m) sum += e;
	EXPECT_EQ(6, sum);
}







TEST(Vector_dense_map, const_test) {

	Dense_Map<int>::BUILDER::Vector::BUILD m;
	m.push_back(1);
	m.push_back(2);
	m.push_back(3);
	m.push_back(4);
	m.push_back(5);

	const auto& c = m;

	EXPECT_EQ(0, c.domain_begin());
	EXPECT_EQ(5, c.domain_end());

	EXPECT_EQ(true, c[2].exists);

	EXPECT_EQ(4, c[3]);
}


TEST(Vector_dense_map, const_accessor_test) {

	Dense_Map<int>::BUILDER::Vector::BUILD m;
	m.push_back(0);
	m.push_back(1);
	m.push_back(2);
	m.push_back(3);
	m.push_back(4);

	//const auto& e = m[2];

	//e = 123;
	//e.val = 123;
	//e.erase();

	//auto val = e.val;
}












TEST(Performance, Vector_dense_map) {
	const int iters = 200;
	const int elements = 100'000;

	double my_time = 0;
	double other_time = 0;

	long long my_result = 0;
	long long other_result = 0;

	for(int iter = 0; iter < iters; ++iter) {

		
		{
			auto t0 = steady_clock::now();


			Dense_Map<int>::BUILDER::Vector::Erasable::BUILD m;

			for(int i=0; i<elements; ++i) m.push_back(i);
			
			//std::random_shuffle(m.begin(), m.end());

			long long result = 0;
			for(const auto& e : m) result += e;
			my_result += result;

			duration<double> dur = steady_clock::now() - t0;
			my_time += dur.count();
		}
		
		{
			auto t0 = steady_clock::now();

			struct Node {
				bool exists = false;
				int value;
			};

			std::vector<Node> m;

			for(int i=0; i<elements; ++i) m.push_back({true,i});

			long long result = 0;
			//for(int i=0; i<elements; ++i) result += m[i].value;
			for(auto e : m) result += e.value;
			other_result += result;

			duration<double> dur = steady_clock::now() - t0;
			other_time += dur.count();

		}
	}


	LOG(INFO) << my_time << "s vs " << other_time << "s";


	EXPECT_EQ(my_result, other_result);

	EXPECT_LT(my_time, other_time*5.5);
}










TEST(Performance, Vector_dense_map_noerase) {
	const int iters = 200;
	const int elements = 100'000;

	double my_time = 0;
	double other_time = 0;

	long long my_result = 0;
	long long other_result = 0;

	for(int iter = 0; iter < iters; ++iter) {
		
		{
			auto t0 = steady_clock::now();

			Dense_Map<int>::BUILDER::Vector::BUILD m;

			for(int i=0; i<elements; ++i) m.push_back(i);
			
			//std::random_shuffle(m.begin(), m.end());

			long long result = 0;
			for(auto e : m) result += e;
			my_result += result;

			duration<double> dur = steady_clock::now() - t0;
			my_time += dur.count();
		}
		
		{
			auto t0 = steady_clock::now();

			std::vector<int> m;

			for(int i=0; i<elements; ++i) m.push_back(i);

			long long result = 0;
			//for(int i=0; i<elements; ++i) result += m[i];
			for(auto e : m) result += e;
			other_result += result;

			duration<double> dur = steady_clock::now() - t0;
			other_time += dur.count();
		}
	}


	LOG(INFO) << my_time << "s vs " << other_time << "s";


	EXPECT_EQ(my_result, other_result);

	//EXPECT_LT(my_time, other_time); // it's actually less... (?!) in g++-7
	EXPECT_LT(my_time, other_time*4); // for clang++-5.0
}




