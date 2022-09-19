#include <iostream>
#include <string>

using namespace std;

struct Point {
	unsigned long long const x, y;

	Point(unsigned long long _x, unsigned long long _y) : x(_x), y(_y) { }

	Point minx(Point const& rha) const {
		return Point(rha.x < x ? rha.x : x, y);
	}
	Point miny(Point const& rha) const {
		return Point(x, rha.y < y ? rha.y : y);
	}
	Point maxx(Point const& rha) const {
		return Point(rha.x > x ? rha.x : x, y);
	}
	Point maxy(Point const& rha) const {
		return Point(x, rha.y > y ? rha.y : y);
	}

	void print() const {
		cout << "(" << x << ", " << y << ") ";
	}
};
ostream& operator<< (ostream& stream, const Point& p) {
	return stream << "(" << p.x << ", " << p.y << ") ";
}


class Rectangle : public Point {
public:
	Rectangle() : Point(0, 0) {}

	Rectangle(Point const& p) : Point(p) {}

	Rectangle operator+(Rectangle const& rha) const {
		return Rectangle(Point(max(x, rha.x), max(y, rha.y)));
	}

	Rectangle operator*(Rectangle const& rha) const {
		return Rectangle(Point(min(x, rha.x), min(y, rha.y)));
	}

	string get_line() {
		return "(" + to_string(x) + "," + to_string(y) + ")";
	}
};


string delete_spaces(string line) { // Удаляет пробелы из строки
	string ans;
	for (int i = 0; i < line.size(); i++) {
		if (line[i] != ' ')
			ans.push_back(line[i]);
	}
	return ans;
}

Rectangle read_rectangle(string& line, int bracket_pos, bool dir) { // Считывает из строки прямоугольник(его и возращает) и стирает его из строки, dir - направление чтения строки
	Rectangle ans();
	int start_pos = bracket_pos, symbols_to_delete = 0;
	int x, y;

	int shift = 1;
	if (!dir)
		shift = -1;

	int curr_pos = bracket_pos + shift;
	string temp;

	// reading first number
	while (line[curr_pos] != ',') {
		temp.push_back(line[curr_pos]);
		curr_pos += shift;
		symbols_to_delete++;
	}
	x = stoi(temp);
	temp.clear();
	curr_pos += shift;

	// reading second number
	while (line[curr_pos] != ')' && line[curr_pos] != '(') {
		temp.push_back(line[curr_pos]);
		curr_pos += shift;
		symbols_to_delete++;
	}
	y = stoi(temp);

	symbols_to_delete += 3;
	if (!dir) {
		line.erase(start_pos - symbols_to_delete + 1, symbols_to_delete);
		return Rectangle(Point(y, x));
	}
	else {
		line.erase(start_pos, symbols_to_delete);
		return Rectangle(Point(x, y));
	}


}

// Эта функция ищет оператор * или + в строке, считывает прямоугольник спрваа и слева от оператора, удаляет это все и вставляет на место результат операции
string one_calculation(string expression) {
	int pos = expression.find("*");

	if (pos != string::npos) {
		Rectangle r1 = read_rectangle(expression, pos + 1, true);
		Rectangle r2 = read_rectangle(expression, pos - 1, false);
		pos = expression.find("*");
		expression.insert(pos + 1, (r1 * r2).get_line());
		expression.erase(pos, 1);
		return expression;
	}

	pos = expression.find("+");
	if (pos != string::npos) {
		Rectangle r1 = read_rectangle(expression, pos + 1, true);
		Rectangle r2 = read_rectangle(expression, pos - 1, false);
		pos = expression.find("+");
		expression.insert(pos + 1, (r1 + r2).get_line());
		expression.erase(pos, 1);
		return expression;
	}

	return expression;
}

int main() {
	string expression, temp_expression;
	getline(cin, expression);



	expression = delete_spaces(expression);
	temp_expression = one_calculation(expression);
	while (expression != temp_expression) { // применяем к строке функцию one_calculation до тех пор, пока ничего не перестанет менятся
		expression = temp_expression;
		temp_expression = one_calculation(expression);
	}
	//cout << expression << endl;

	Rectangle ans = read_rectangle(expression, expression.find("("), true);
	cout << "answer output with cout: " << ans << endl;
}