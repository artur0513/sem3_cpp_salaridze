#include <iostream>
#include <cassert>

using namespace std;


template<class T>class Grid_line {
private:
	T** data;
public:
	Grid_line(T** data) : data(data) {}

	T& operator[](int i) {
		T& t = *data[i];
		return t;
	}
};

template<class T>class Grid final {
private:
	using value_type = T;

public:
	using size_type = unsigned;
	T* data;
	size_type y_size, x_size;

	Grid(T* data, size_type y_size, size_type x_size) :
		data(data), y_size(y_size), x_size(x_size) { }

	Grid(T const& t) {
		data = new T[1];
		x_size = 1;
		y_size = 1;
		data[0] = t;
	}

	Grid(size_type _y_size, size_type _x_size, T const& t) {
		y_size = _y_size;
		x_size = _x_size;
		data = new T[y_size * x_size];
		*this = t;
	}

	Grid(size_type _y_size, size_type _x_size) {
		Grid(y_size, _x_size, T());
	}

	Grid(Grid<T> const& g) {
		cout << "copy" << endl;
		y_size = g.y_size;
		x_size = g.x_size;
		data = new T[y_size * x_size];
		for (auto it = data, end = data + x_size * y_size; it != end; it++)
			*it = g.data[it - data];

	}

	Grid<T>& operator=(Grid<T>& g) {
		cout << "copy=" << endl;
		delete[] data;
		y_size = g.y_size;
		x_size = g.x_size;
		data = new T[y_size * x_size];
		for (auto it = data, end = data + x_size * y_size; it != end; it++)
			*it = g.data[it - data];
		return *this;
	}

	Grid(Grid<T>&& g) : data(g.data), y_size(g.y_size), x_size(g.x_size) {
		cout << "move" << endl;
		g.data = nullptr;
	};

	Grid<T>& operator=(Grid<T>&& g) {
		cout << "move=" << endl;
		y_size = g.y_size;
		x_size = g.x_size;
		delete[] data;
		data = g.data;
		g.data = nullptr;
		return *this;
	};

	~Grid() {
		delete[] data;
	}

	T operator() (size_type y_idx, size_type x_idx) const {
		return data[y_idx * x_size + x_idx];
	}

	T& operator() (size_type y_idx, size_type x_idx) {
		return data[y_idx * x_size + x_idx];
	}

	Grid<T>& operator=(T const& t) {
		for (auto it = data, end = data + x_size * y_size; it != end; it++)
			*it = t;
		return *this;
	}

	size_type get_x_size() const {
		return x_size;
	}

	size_type get_y_size() const {
		return y_size;
	}

	void print() {
		for (auto it = data, end = data + x_size * y_size; it != end; it++) {
			if ((it - data) % x_size == 0)
				cout << endl;
			cout << *it << " ";
		}
		cout << endl;
	}

	Grid_line<T> operator[](size_type x) {
		T** mas = new T * [y_size];
		for (size_type i = 0; i < y_size; i++) {
			mas[i] = data + x + i * y_size;
		}
		return Grid_line<T>(mas);
	}

};

Grid<int> for_test() {
	return Grid<int>(1, 2, 3);
}

int main() {
	Grid<int> g1(3, 5, 10);
	Grid<int> g2 = g1;
	g2 = Grid<int>(2, 2, 123);
	cout << g2[1][0] << endl;
	g2.print();

	cout << "========" << endl;

	Grid<int> g3(0);
	Grid<int> g4 = std::move(g3);
	g4.print();

	cout << "========" << endl;

	Grid<float> g(3, 2, 0.0f);
	assert(3 == g.get_y_size());
	assert(2 == g.get_x_size());

	using gsize_t = Grid<int>::size_type;
	for (gsize_t y_idx = 0; y_idx != g.get_y_size(); ++y_idx)
		for (gsize_t x_idx = 0; x_idx != g.get_x_size(); ++x_idx)
			assert(0.0f == g[y_idx][x_idx]);


	for (gsize_t y_idx = 0; y_idx != g.get_y_size(); ++y_idx)
		for (gsize_t x_idx = 0; x_idx != g.get_x_size(); ++x_idx)
			g[y_idx][x_idx] = 1.0f;

	for (gsize_t y_idx = 0; y_idx != g.get_y_size(); ++y_idx)
		for (gsize_t x_idx = 0; x_idx != g.get_x_size(); ++x_idx)
			assert(1.0f == g(y_idx, x_idx));
	return 0;

}
