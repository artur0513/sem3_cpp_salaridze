#include <iostream>

using namespace std;

template <class T> struct Comparator {
	virtual bool operator() (T const&, T const&) const = 0;
};

struct IntComparator final : Comparator<int> {
	bool operator() (int const& lha, int const& rha) const override {
		return lha < rha;
	}
};

template<class T> size_t partition(T* data, size_t r, Comparator<T>& comp) {
	size_t l = 0;
	T support_element = data[l];

	while (true) {
		while (comp(data[l], support_element) && l < r)
			l++;
		while (!comp(data[r], support_element) && r > 0)
			r--;
		if (r <= l)
			return r;

		swap(data[l++], data[r--]);
	}
}

template<class T> void quicksort(T* data, size_t size, Comparator<T>& comp) {
	if (size < 1)
		return;
	size_t split = partition(data, size, comp);
	quicksort(data, split, comp);
	quicksort(data + split + 1, size - split - 1, comp);
}

bool test(int size, bool print = true) {
	// if print == true to pechataem v consol massiv
	int* k = new int[size];

	if (print)
		cout << "test na massive iz " << size << " elementov" << endl;

	srand(time(0));
	for (int i = 0; i < size; i++) {
		k[i] = rand();
		if (print)
			cout << k[i] << " ";
	}
	if (print)
		cout << endl;

	IntComparator comp;
	quicksort(k, size - 1, comp);

	bool correct_sort = true;
	for (int i = 0; i < size; i++) {
		if (i != 0) {
			if (k[i] < k[i - 1])
				correct_sort = false;
		}
		if (print)
			cout << k[i] << " ";
	}
	if (print)
		cout << endl;
	delete[] k;
	return correct_sort;
}

int main() {
	if (test(20))
		cout << "correct" << endl;
	else
		cout << "wrong" << endl;

	cout << endl << "test na 10000 elementov (1 = correct, 0 = wrong): " << test(10000, false) << endl;
}
/*

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
		int i = heap_size, parent = (i - 1)/2;
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
}*/