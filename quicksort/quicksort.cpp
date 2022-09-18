#include <iostream>

using namespace std;

template<class T> size_t partition(T* data, size_t r) {
	size_t l = 0;
	size_t support_element = (data[l] + data[r]) / 2;

	while (true) {
		while (data[l] < support_element && l < r)
			l++;
		while (data[r] > support_element && r > 0)
			r--;
		if (r <= l)
			return r;

		swap(data[l++], data[r--]);
	}
}

template<class T> void quicksort(T* data, size_t size) {
	if (size < 1)
		return;
	size_t split = partition(data, size);
	quicksort(data, split);
	quicksort(data + split + 1, size - split - 1);
}

bool test(int size, bool print = true) {
	// if print == true to pechataem v consol massiv
	int* k = new int[size];

	if (print)
		cout << "test na massive iz " << size << " elementov" << endl;

	srand(time(0));
	for (int i = 0; i < size; i++) {
		k[i] = rand() % 10000;
		if (print)
			cout << k[i] << " ";
	}
	if (print)
		cout << endl;

	quicksort(k, size - 1);

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