
#include "3rd/Catch/single_include/catch.hpp"
#include "ara/dlist.h"

class MyData : public ara::dlist<MyData>
{
public:
	MyData(int n = 0) : data_(n) {}
	int	data_;
};

TEST_CASE("double list", "[base]") {

	MyData		root;
	root.as_root();

	MyData		node1(1);
	node1.append_after(root);	//root ->  node1
	
	MyData		node2(2);
	node2.append_after(node1);	//root -> node1 -> node2

	MyData		node3(3);
	node3.append_after(node1);	//root -> node1 -> node3 -> node2

	MyData		node4(4);
	node4.append_before(root);	//root -> node1 -> node3 -> node2 -> node4 

	MyData		node5(5);
	node5.append_before(node3);	//root -> node1 -> node5 -> node3 -> node2 -> node4 

	int		data_val[] = { 1, 5, 3, 2, 4 };
	size_t count = sizeof(data_val) / sizeof(data_val[0]);
	int c = 0;
	for (MyData * p = root.get_next(); p != &root; p = p->get_next(), ++c) {
		REQUIRE(data_val[c] == p->data_);
	}
	REQUIRE(c == count);

	node4.unlink();
	--count;
	c = 0;
	for (MyData * p = root.get_next(); p != &root; p = p->get_next(), ++c) {
		REQUIRE(data_val[c] == p->data_);
	}
	REQUIRE(c == count);
}
