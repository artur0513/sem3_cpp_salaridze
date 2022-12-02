#include <iostream>
#include <cassert>
#include <stdarg.h>

using namespace std;

template<class T, int dim> class Grid final {
private:
	using value_type = T;

public:
	using size_type = unsigned;
	T* data;

	size_type* sizes;
	size_type data_size = 1;

	Grid(T* data, size_type first_size, ...) : data(data) {
		va_list ap;
		va_start(ap, first_size);
		sizes = new size_type[dim];

		sizes[0] = first_size;
		data_size = first_size;
		for (int i = 1; i < dim; i++) {
			sizes[i] = va_arg(ap, size_type);
			data_size *= sizes[i];
		}
		va_end(ap);
	}

	Grid(T* data, size_type* sizes) : data(data), sizes(sizes) {
		for (int i = 0; i < dim; i++) {
			data_size *= sizes[i];
		}
	}

	Grid(T const& t) {
		data = new T[1];
		data[0] = t;
		sizes = new size_type[dim];
		for (int i = 0; i < dim; i++) {
			sizes[i] = 1;
		}
	}

	Grid(T const& t, size_type first_size, ...) {
		va_list ap;
		va_start(ap, first_size);
		sizes = new size_type[dim];

		sizes[0] = first_size;
		data_size = first_size;
		for (int i = 1; i < dim; i++) {
			sizes[i] = va_arg(ap, size_type);
			data_size *= sizes[i];
		}
		va_end(ap);

		data = new T[data_size];
		*this = t;
	}

	Grid(size_type first_size, ...) {
		va_list ap;
		va_start(ap, first_size);
		sizes = new size_type[dim];

		sizes[0] = first_size;
		data_size = first_size;
		for (int i = 1; i < dim; i++) {
			sizes[i] = va_arg(ap, size_type);
			data_size *= sizes[i];
		}
		va_end(ap);

		data = new T[data_size];
		*this = T();
	}

	Grid(Grid<T, dim> const& g) {
		cout << "copy" << endl;
		sizes = new size_type[dim];
		data_size = g.data_size;
		for (int i = 0; i < dim; i++) {
			sizes[i] = g.sizes[i];
		}
		data = new T[g.data_size];
		for (auto it = data, end = data + data_size; it != end; it++)
			*it = g.data[it - data];
	}

	Grid<T, dim>& operator=(Grid<T, dim>& g) {
		cout << "copy=" << endl;
		delete[] data;
		delete[] sizes;
		data_size = g.data_size;
		sizes = new size_type[dim];
		for (int i = 0; i < dim; i++) {
			sizes[i] = g.sizes[i];
		}
		data = new T[g.data_size];
		for (auto it = data, end = data + data_size; it != end; it++)
			*it = g.data[it - data];
		return *this;
	}

	Grid(Grid<T, dim>&& g) {
		cout << "move" << endl;
		data = g.data;
		sizes = g.sizes;
		data_size = g.data_size;

		g.data = nullptr;
		g.sizes = nullptr;
	};

	Grid<T, dim>& operator=(Grid<T, dim>&& g) {
		cout << "move=" << endl;
		delete[] data;
		delete[] sizes;

		data = g.data;
		sizes = g.sizes;
		//cout << g.sizes[0] << endl;
		data_size = g.data_size;

		g.data = nullptr;
		g.sizes = nullptr;
		return *this;
	};

	~Grid() {
		delete[] data;
		delete[] sizes;
	}

	size_type coord_to_pos(size_type* coord) {
		size_type pos = 0;
		for (size_type i = 0; i < dim; i++) {
			size_type weight = 1;
			for (size_type j = dim - 1; j > i; j--) {
				weight *= sizes[j];
			}
			pos += coord[i] * weight;
		}
		return pos;
	}

	T operator() (size_type first_coord, ...) const {
		va_list ap;
		va_start(ap, first_coord);
		size_type* coord = new size_type[dim];

		coord[0] = first_coord;
		for (int i = 1; i < dim; i++) {
			coord[i] = va_arg(ap, size_type);
		}
		va_end(ap);

		size_type pos = coord_to_pos(coord);
		delete[] coord;

		return data[pos];
	}

	T& operator() (size_type first_coord, ...) {
		va_list ap;
		va_start(ap, first_coord);
		size_type* coord = new size_type[dim];

		coord[0] = first_coord;
		for (int i = 1; i < dim; i++) {
			coord[i] = va_arg(ap, size_type);
		}
		va_end(ap);

		size_type pos = coord_to_pos(coord);
		delete[] coord;

		return data[pos];
	}

	Grid<T, dim>& operator=(T const& t) {
		for (auto it = data, end = data + data_size; it != end; it++) {
			*it = t;
		}
		return *this;
	}

	void print() {
		cout << "sizes: ";
		for (int i = 0; i < dim; i++) {
			cout << sizes[i] << " ";
		}
		cout << endl << "data: ";
		for (int i = 0; i < data_size; i++) {
			cout << data[i] << " ";
		}
		cout << endl;
	}

	Grid<T, dim - 1> operator[](size_type x) {
		size_type new_data_size = data_size / sizes[0];
		T* mas = new T[new_data_size];
		for (size_type i = 0; i < new_data_size; i++) {
			mas[i] = *(data + x + i * sizes[0]);
		}
		size_type* new_sizes = new size_type[dim - 1];
		for (size_type i = 0; i < dim - 1; i++) {
			new_sizes[i] = sizes[i + 1];
		}
		return Grid<T, dim - 1>(mas, new_sizes);
	}
};

int main() {
	float* data = new float[81];
	for (int i = 0; i < 81; i++) {
		data[i] = float(i);
	}
	Grid<float, 4> g(data, 3u, 3u, 3u, 3u);
	//g.print();

	unsigned* test = new unsigned[4];
	test[0] = 1;
	test[1] = 1;
	test[2] = 1;
	test[3] = 1;

	Grid<float, 3> g3(1.0f, 2, 3, 4);
	g3.print();
	assert(1.0f == g3(1, 1, 1));
	Grid<float, 2> g2(2.0f, 2, 5);
	assert(2.0f == g2(1, 1));
	g2 = g3[1];
	assert(1.0f == g2(1, 1));
}
