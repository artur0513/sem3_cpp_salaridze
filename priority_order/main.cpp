#include <iostream>
#include <vector>
#include <deque>
#include <utility>

using namespace std;

template<class T> class Queue {
public:
	deque<pair<T, int>> items;

	void add(T new_item, int prior) {
		for (int i = 0; i < items.size(); i++) {
			if (prior > items[i].second) {
				items.insert(items.begin() + i, pair<T, int>(new_item, prior));
				return;
			}
		}
		items.push_back(pair<T, int>(new_item, prior));
	}

	void pop_back() {
		if (items.size() > 0) {
			items.pop_front();
		}
	}

	void print() {
		for (int i = 0; i < items.size(); i++) {
			cout << items[i].first << " " << items[i].second << endl;
		}
		cout << endl;
	}
};

int main() {
	Queue<string> q;
	q.add("aaa", 10);
	q.add("bbb", 15);
	q.add("ccc", 25);
	q.add("ddd", 20);
	q.add("sodfn", 3);
	q.print();
	q.pop_back();
	q.print();
}