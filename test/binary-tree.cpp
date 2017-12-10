#include <salgo/binary-tree.hpp>
#include <salgo/inorder.hpp>

#include <gtest/gtest.h>

//#include <chrono>

using namespace salgo;

//using namespace std::chrono;





TEST(Implicit_binary_tree, simple_1) {

	Binary_Tree_Builder<int>::Add_Flags<IMPLICIT>::Build tree;

	EXPECT_FALSE(tree.root);
	EXPECT_FALSE(tree.root().exists);

	tree.root() = 42;

	EXPECT_TRUE(tree.root().exists);
	EXPECT_EQ(42, tree.root());
}





TEST(Implicit_binary_tree, simple_2) {

	Binary_Tree_Builder<int>::Add_Flags<IMPLICIT | BT_VERTS_ERASABLE>::Build tree;

	EXPECT_FALSE(tree.root().exists);
	tree.root() = 1;
	EXPECT_TRUE(tree.root().exists);

	tree.root().right() = 11;
	tree.root().right().right() = 111;

	EXPECT_TRUE(tree.root().exists);
	EXPECT_TRUE(tree.root().right().exists);
	EXPECT_TRUE(tree.root().right().right().exists);

	EXPECT_FALSE(tree.root().right().left().exists);

	EXPECT_EQ(1, tree.root());
	EXPECT_EQ(11, tree.root().right());
	EXPECT_EQ(111, tree.root().right().right());
}



TEST(Implicit_binary_tree, traverse_1) {

	Binary_Tree_Builder<int>::Add_Flags<IMPLICIT | BT_VERTS_ERASABLE>::Build tree;

	tree.root = 1;
	tree.root().left = 2;
	tree.root().right = 3;
	tree.root().right().left = 3;
	tree.root().right().right() = 3;

	auto sum = 0;
	int num = 0;
	for(auto e : tree) {
		sum += e;
		++num;
	}
	EXPECT_EQ(12, sum);
	EXPECT_EQ(5, num);
}



TEST(Implicit_binary_tree, inorder_1) {

	Binary_Tree_Builder<int>::Add_Flags<IMPLICIT | BT_VERTS_ERASABLE>::Build tree;

	tree.root() = 1;
	tree.root().left() = 2;
	tree.root().right() = 3;
	tree.root().right().left() = 4;
	tree.root().right().right() = 5;

	auto sum = 0;

	std::vector<int> vals;

	for(auto e : Inorder(tree)) {
		sum += e;
		vals.push_back(e);
	}

	EXPECT_EQ(15, sum);


	ASSERT_EQ(5, vals.size());

	EXPECT_EQ(2, vals[0]);
	EXPECT_EQ(1, vals[1]);
	EXPECT_EQ(4, vals[2]);
	EXPECT_EQ(3, vals[3]);
	EXPECT_EQ(5, vals[4]);
}






TEST(Binary_tree, inorder_1) {

	Binary_Tree<int> tree;

	{
		auto root = tree.add();

		root = 1;
		root.left.create() = 2;
		EXPECT_FALSE(root.parent);
		EXPECT_TRUE(root.left().exists);
		EXPECT_TRUE(root.left);

		root.right.create(3).left.create(4).parent().right.create(5);
	}

	auto sum = 0;

	std::vector<int> vals;

	for(auto e : Inorder(tree)) {
		sum += e;
		vals.push_back(e);
	}

	EXPECT_EQ(15, sum);


	ASSERT_EQ(5, vals.size());

	EXPECT_EQ(2, vals[0]);
	EXPECT_EQ(1, vals[1]);
	EXPECT_EQ(4, vals[2]);
	EXPECT_EQ(3, vals[3]);
	EXPECT_EQ(5, vals[4]);
}




TEST(Binary_Tree, check_link) {

	Binary_Tree_Builder<>::Add_Flags<PARENT_LINKS>::Build tree;

	auto root = tree.add();

	EXPECT_FALSE(root.right);
}




