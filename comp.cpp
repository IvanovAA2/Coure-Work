#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

int main() {
	time_point<steady_clock> start = steady_clock::now();
	double result = 0;
	int i = 1, j;
	while(i < 10000) {
		j = 1;
		while(j < 10000) {
			result += 1. / i / j;
			j = j + 1;
		}
		i = i + 1;
	}
	cout << duration<double, milliseconds::period>(steady_clock::now() - start).count() << endl;
	cout << result << endl;
}