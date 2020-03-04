#include "Mesh.h"
#include "Iterators.h"
#include <iostream>

//Please test this program using "net.obj", in which the corner angles are either 45 or 90 degrees.

void ComputeFaceCornerAngles(Face * f, double cAngles[3]) {
	Halfedge * hes[3];
	hes[0] = f->he();
	hes[1] = hes[0]->next();
	hes[2] = hes[0]->prev();
	Point & p0 = hes[0]->target()->point();
	Point & p1 = hes[1]->target()->point();
	Point & p2 = hes[2]->target()->point();
	double l20 = (p0 - p2).norm();
	double l01 = (p1 - p0).norm();
	double l12 = (p2 - p1).norm();
	cAngles[0] = acos((l20*l20 + l01*l01 - l12*l12) / (2*l20*l01)); 
	cAngles[1] = acos((l01*l01 + l12*l12 - l20*l20) / (2*l01*l12));
	cAngles[2] = acos((l12*l12 + l20*l20 - l01*l01) / (2*l12*l20));	
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

	std::cout << "Calculating all the corner angles of this mesh.\n";

	double PI = 3.14159265358979;
	double factorRadiusToDegree = 180 / PI;

	std::vector<double> cornerAngles;
	cornerAngles.resize(cMesh->numFaces() * 3, 0);
	std::vector<Halfedge*> corners; //a corner can be uniquely indexed using a half-edge
	for (MeshFaceIterator fit(cMesh); !fit.end(); ++fit) {
		Face * f = *fit;
		Halfedge * he1 = f->he();
		Halfedge * he2 = he1->next();
		Halfedge * he3 = he2->next();
		double fAngles[3];
		ComputeFaceCornerAngles(f, fAngles);
		cornerAngles[he1->index()] = fAngles[0];
		cornerAngles[he2->index()] = fAngles[1];
		cornerAngles[he3->index()] = fAngles[2];
		std::cout << "Face " << f->index() << "\n";
		std::cout << "Corners and their angles: \n";
		std::cout << "Angle (" << he1->source()->index() << "," << he1->target()->index() << "," << he2->target()->index() << ") = " << fAngles[0] * factorRadiusToDegree << "\n";
		std::cout << "Angle (" << he2->source()->index() << "," << he2->target()->index() << "," << he3->target()->index() << ") = " << fAngles[1] * factorRadiusToDegree << "\n";
		std::cout << "Angle (" << he3->source()->index() << "," << he3->target()->index() << "," << he1->target()->index() << ") = " << fAngles[2] * factorRadiusToDegree << "\n";
	}

	delete cMesh;
	system("pause");
	return 0;
}
