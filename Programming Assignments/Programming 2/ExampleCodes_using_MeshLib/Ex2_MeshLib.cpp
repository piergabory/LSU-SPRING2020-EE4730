#include "Mesh.h"
#include "Iterators.h"
#include <iostream>

double ComputeFaceArea(Face * f) {
	Halfedge * he1 = f->he(); // one halfedge
	Halfedge * he2 = he1->next(); // its next halfedge inside this face
	Point & pt1 = he1->source()->point();
	Point & pt2 = he1->target()->point();
	Point & pt3 = he2->target()->point();
	Point p21 = pt2 - pt1;
	Point p31 = pt3 - pt1;
	Point cprod = p21 ^ p31;
	return cprod.norm() / 2.0 ;
}


int main(int argc, char ** argv) {
	Mesh * cMesh = new Mesh();

	if (argc != 2) {
		std::cerr << "Provide an obj file to test the meshlib.\n";
		return 1;
	}

	bool flag = cMesh->readOBJFile(argv[1]);

	if (!flag) {
		std::cerr << "Fail to read mesh " << argv[1] << ".\n";
		return -1;
	}

	std::cout << "Calculating the area of this mesh.\n";

	std::vector<double> faceAreas;
	double sumArea = 0;
	for (MeshFaceIterator fit(cMesh); !fit.end(); ++fit) {
		Face * f = *fit;
		double area = ComputeFaceArea(f);
		if (f->index() != faceAreas.size()) {
			std::cerr << "Face index mismatched the way we store the areas!";
			break;
		}
		sumArea += area;
		faceAreas.push_back(area);  
		//now this face's area can be retrieved from faceAreas[f->index()]		
	}
	std::cout << "The area of this mesh is " << sumArea << "\n";

	delete cMesh;
	system("pause");
	return 0;
}