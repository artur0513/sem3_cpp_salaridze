#include <iostream>
#include <vector>
#include <deque>
#include <utility>
#include <list>

using namespace std;

template <class T> struct Comparator {
	virtual bool operator() (T const&, T const&) const = 0;
};

struct IntComparator final : Comparator<int> {
	bool operator() (int const& lha, int const& rha) const override {
		return lha < rha;
	}
};

struct StringComparator final : Comparator<string> {
	bool operator() (string const& lha, string const& rha) const override {
		return lha.size() > rha.size();
	}
};

template<class T> class Heap {
private:
	Comparator<T>& comp;
	T* arr;
	const size_t max_size = 1000;
	size_t heap_size = 0;

public:
	Heap(Comparator<T>& _comp) : comp(_comp) {
		arr = new T[max_size];
	}

	void push(T item) {
		int i = heap_size, parent = (i - 1) / 2;
		arr[i] = item;

		while (parent >= 0 && i > 0) {
			if (comp(arr[i], arr[parent])) {
				T temp = arr[i];
				arr[i] = arr[parent];
				arr[parent] = temp;
			}
			i = parent;
			parent = (i - 1) / 2;
		}
		heap_size++;
	}

	void heapify(int i) {
		int left_child, right_child, biggest;
		T temp;

		while (true) {
			left_child = 2 * i + 1;
			right_child = 2 * i + 2;
			biggest = i;

			if (left_child < heap_size) {
				if (comp(arr[left_child], arr[biggest])) {
					biggest = left_child;
				}
			}
			if (right_child < heap_size) {
				if (comp(arr[right_child], arr[biggest])) {
					biggest = right_child;
				}
			}
			if (biggest == i)
				return;

			temp = arr[i];
			arr[i] = arr[biggest];
			arr[biggest] = temp;
			i = biggest;
		}
	}

	void poll() {
		if (heap_size == 0)
			return;

		arr[0] = arr[heap_size - 1];
		heap_size--;
		heapify(0);
	}

	T peek() {
		return arr[0];
	}

	bool is_empty() {
		return heap_size == 0;
	}

	void free() {
		delete[] arr;
	}
};


int main() {
	StringComparator c;
	Heap<string> q(c);

	q.push("asd");
	q.push("bsadfbbkjnedvdbe");
	q.push("cc");
	q.push("d");
	q.push("sodoeihgfn");
	cout << q.peek() << endl;
	q.poll();
	cout << q.peek() << endl;
	q.push("qqqqqqqqqqqqqqqqqqqqq");
	cout << q.peek() << endl;

	q.poll();
	cout << q.peek() << endl;

	q.poll();
	cout << q.peek() << endl;

	q.poll();
	cout << q.peek() << endl;

	q.free();
}