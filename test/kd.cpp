#include <salgo/kd.hpp>

#include <gtest/gtest.h>

#include <chrono>
using namespace std::chrono;

using namespace salgo;







using Vec4 = Eigen::Matrix<double, 4, 1>;
using Vec4_noalign = Eigen::Matrix<double, 4, 1, Eigen::DontAlign>;



template<class KD>
void insert_grid(KD& kd) {

	std::vector<Vec4_noalign> v;

	for(int x=0; x<10; ++x) {
		for(int y=0; y<10; ++y) {
			for(int z=0; z<10; ++z) {
				for(int w=0; w<10; ++w) {
					v.emplace_back(x,y,z,w);
				}
			}
		}
	}

	std::random_shuffle(v.begin(), v.end());

	for(auto& p : v) {
		kd.add(p);
	}
}

template<class KD>
void insert_grid_keyval(KD& kd) {

	std::vector<std::pair<Vec4_noalign,int>> v;

	for(int x=0; x<5; ++x) {
		for(int y=0; y<5; ++y) {
			for(int z=0; z<5; ++z) {
				for(int w=0; w<5; ++w) {
					v.emplace_back(Vec4_noalign{1.0*x, 1.0*y, 1.0*z, 1.0*w}, 1000*x + 100*y + 10*z + w);
				}
			}
		}
	}

	std::random_shuffle(v.begin(), v.end());

	for(auto& p : v) {
		kd.add(p.first, p.second);
	}
}







/*
TEST(Kd, Each_aabb) {
	Kd<double, 4> :: BUILDER :: BUILD kd;

	insert_grid(kd);

	std::vector<Vec4_noalign> v;

	auto box = Eigen::AlignedBox<double, 4>(
		Vec4{1.5, 1.5, 1.5, 1.5},
		Vec4{3.5, 3.5, 3.5, 3.5}
	);

	kd.each_intersect(box, [&v](const auto& e){
		v.emplace_back( e.key() );
	});

	ASSERT_EQ(16, v.size());

	Eigen::Matrix<double, 4, 1> r = {0,0,0,0};
	for(auto& e : v) {
		r += e;
	}
	r /= v.size();

	EXPECT_FLOAT_EQ(2.5, r[0]);
	EXPECT_FLOAT_EQ(2.5, r[1]);
	EXPECT_FLOAT_EQ(2.5, r[2]);
	EXPECT_FLOAT_EQ(2.5, r[3]);
}
*/





TEST(Kd_aabb, Each_aabb) {
	srand(69);

	using Aabb = Eigen::AlignedBox<double, 4>;
	Kd<double, 4> :: BUILDER :: Key<Aabb> :: BUILD kd;

	std::vector<Aabb> v;

	for(int i=0; i<100000; ++i) {
		Vec4 fr = {rand()%10 * 1.0, rand()%10 * 1.0, rand()%10 * 1.0, rand()%10 * 1.0};
		Vec4 to = {rand()%10 * 1.0, rand()%10 * 1.0, rand()%10 * 1.0, rand()%10 * 1.0};
		auto new_fr = fr.array().min(to.array()).matrix();
		auto new_to = to.array().max(fr.array()).matrix();
		v.emplace_back(new_fr, new_to);
	}



	{
		auto t0 = steady_clock::now();

		for(auto& bb : v) {
			kd.add(bb);
		}
		
		duration<double> dur = steady_clock::now() - t0;
		LOG(INFO) << "insertion time: " << dur.count();
	}



	auto query_bb = Eigen::AlignedBox<double, 4>(Vec4{1.5, 1.5, 1.5, 1.5}, Vec4{3.5, 3.5, 3.5, 3.5});

	int num = 0;
	{
		auto t0 = steady_clock::now();

		kd.each_intersect(query_bb, [&num](auto) {
			++num;
		});

		duration<double> dur = steady_clock::now() - t0;
		LOG(INFO) << "query time: " << dur.count();
	}

	int true_num = 0;
	for(auto& bb : v) {
		if(bb.intersects(query_bb)) ++true_num;
	}

	LOG(INFO) << "true_num: " << true_num;

	EXPECT_EQ(true_num, num);
}



/* TODO:

TEST(Kd, FindClosest) {
	Kd<double, 4> :: BUILDER :: BUILD kd;

	insert_grid(kd);

	auto v = kd.find_closest({1.49, 3.88, 0.12, 2.7});
	ASSERT(v.exists);
	EXPECT_EQ(Vec4(1,4,0,3), v.key);
}


TEST(Kd, FindClosestIf) {
	Kd<double, 4> :: BUILDER :: BUILD kd;

	insert_grid(kd);

	auto v = kd.find_closest_if({0.49, 3.88, 1.12, 2.8}, [](auto& x){return x[1] < 3.9;});
	ASSERT_NE(nullptr, v);
	EXPECT_EQ(Vec4(0,3,1,3), *v);
}



TEST(Kd, FindClosestVal) {
	Kd<double, 4> :: BUILDER :: Val<int> :: BUILD kd;

	insert_grid_keyval(kd);

	auto v = kd.find_closest({1.12, 2.9, 3.9, 0.8});
	ASSERT_NE(nullptr, v);
	EXPECT_EQ(1000*1 + 100*3 + 10*4 + 1, v->val);
}


TEST(Kd, FindKClosest) {
	Kd<double, 4> kd;

	insert_grid(kd);

	auto v = kd.find_k_closest(Vec4{3.4, 3.6, 1.4, 0.1}, 8);
	ASSERT_EQ(8, v.size());

	Eigen::Matrix<double, 4, 1> r = {0,0,0,0};
	for(auto& e : v) {
		r += *e;
	}
	r /= v.size();

	EXPECT_FLOAT_EQ(3.5, r[0]);
	EXPECT_FLOAT_EQ(3.5, r[1]);
	EXPECT_FLOAT_EQ(1.5, r[2]);
	EXPECT_FLOAT_EQ(0.0, r[3]);
}


TEST(Kd, FindKClosestIf) {
	Kd<double, 4> kd;

	insert_grid(kd);

	auto v = kd.find_k_closest_if(Vec4{3.4, 3.7, 1.3, 0.3}, 8, [](auto& x){return x[1] < 3.9;});
	ASSERT_EQ(8, v.size());

	Eigen::Matrix<double, 4, 1> r = {0,0,0,0};
	for(auto& e : v) {
		r += *e;
	}
	r /= v.size();

	EXPECT_FLOAT_EQ(3.5, r[0]);
	EXPECT_FLOAT_EQ(3.0, r[1]);
	EXPECT_FLOAT_EQ(1.5, r[2]);
	EXPECT_FLOAT_EQ(0.5, r[3]);
}

*/



