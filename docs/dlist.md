# dlist

## [Header file](../ara/dlist.h)

~~~C++
#include "ara/dlist.h"
~~~

## Descriptions

C++ STL has a doubly linked list template -- std::list.
The doubly-linked-list container deletes item by **iterator**.
Sometimes it's bother to store the **iterator** in the object, while the object wants to delete self from the list.

**dlsit** is a simple template to help inject two pointers into a class, and implements some container functions such as append/delete/navigate.

## Example

~~~C++
class MyData : public ara::dlist<MyData>
{
public:
    MyData(int n = 0) : data_(n) {}
    int data_;
};

MyData root;
root.as_root(); //It's important to initiate the root node.

MyData node1(1);
node1.append_after(root); // root ->  node1 -> root

MyData node2(2);
node2.append_after(root); // root ->  node2 -> node1 -> root

MyData node3(3);
node3.append_before(root); // root ->  node2 -> node1 -> node3 -> root
node3.unlink(); // root ->  node2 -> node1 -> root

for (MyData & d : root) {
    //handle data with d.
}

for (const MyData & d2 : ara::reverse_range(root_c)) {
    //handle data with d2.
}
~~~

## Notes

It's important to initiate the root node by call member function **as_root()**. The root node's pointers are both point to root node self. And the normal node's pointers are both point to nullptr.

## See also

* test and exsample :
  * [test/test_dlist.cpp](../test/test_dlist.cpp)
* other classes used it:
  * [ara/async_queue.h](../ara/async_queue.h)
  * [ara/async_rwqueue.h](../ara/async_rwqueue.h)