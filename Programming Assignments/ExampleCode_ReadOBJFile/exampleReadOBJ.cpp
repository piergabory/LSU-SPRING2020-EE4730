#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

bool ReadFile(const char filename[]) {	
	std::cout << "Reading a File " << filename << " ...";
	std::ifstream input(filename);
	if (!input.good()) {
		std::cerr << "Can't open file " << filename << "!" << std::endl;
		return false;
	}
	int numV, numF;
	numV = 0; //total number of vertices
	numF = 0; //total number of faces;
	std::cout << "\n";
	while (input.good()) {
		std::string line;
		std::getline(input, line);
		if (line.empty()) continue;
		std::stringstream ss(line);
		std::string keyword;
		ss >> keyword;
		if (keyword == "v") {	//parsing a line of vertex element					
			double x, y, z;
			ss >> x >> y >> z;
			//here we have got the three coordinates 
			std::cout << x << " " << y << " " << z << "\n";
			numV++;
		}
		if (keyword == "f") {	//parsing a line of face element					
			int v1, v2, v3;
			ss >> v1 >> v2 >> v3;
			//here we have got the three vertex indeces;  but the index starts with 1
			std::cout << "face (";
			std::cout << "v" << v1 - 1 << " ";
			std::cout << "v" << v2 - 1 << " ";
			std::cout << "v" << v3 - 1 << ")\n";
			numF++;
		}
	}
	std::cout << "Totally " << numV << " vertices, and " << numF << " faces.\n";
	return true;
}

int main(int argc, char** argv) {
	if (argc == 2) {
		ReadFile(argv[1]);
	}
	system("pause");
	return 1;
}