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