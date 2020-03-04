#include "../MeshLib_Core/Mesh.h"
#include "../MeshLib_Core/Iterators.h"
#include <iostream>

int main(int argc, char ** argv) {
	Mesh * cMesh = new Mesh();

	if (argc != 2) {
		std::cerr << "Provide an obj file to test the meshlib.\n";
		system("pause");
		return 1;
	}

	bool flag = cMesh->readOBJFile(argv[1]);

	if (!flag) {
		std::cerr << "Fail to read mesh " << argv[1] << ".\n";
		system("pause");
		return -1;
	}

	std::cout << "Total Number of Faces: " << cMesh->numFaces() << "\n";

	std::cout << "Counting Face Number ... ";
	int fNum = 0;
	for (MeshFaceIterator fit(cMesh); !fit.end(); ++fit) {
		Face * f = *fit;
		fNum++;
	}
	std::cout << "fNum = " << fNum << "\n";

	std::cout << "Total Number of Vertices: " << cMesh->numVertices() << "\n";
	std::cout << "Total Number of Edges: " << cMesh->numEdges() << "\n\n";

	for (MeshVertexIterator vit(cMesh); !vit.end(); ++vit) {
		Vertex * ver = *vit;
		std::cout << "Vertex " << ver->index() << " (" << ver->point()[0] << ", "
			<< ver->point()[1] << ", " << ver->point()[2] << ")\n";
		std::cout << "It is ";
		if (!ver->boundary())
			std::cout << "not ";
		std::cout << "a boundary vertex.\n";
			
		std::cout << "Its 1-ring neighboring vertices are: \n";
		for (VertexVertexIterator vvit(ver); !vvit.end(); ++vvit) {
			Vertex * nv = *vvit;
			std::cout << "Vertex " << nv->index() << " (" << nv->point()[0] << ", "
				<< nv->point()[1] << ", " << nv->point()[2] << ")\n";
		}		
		std::cout << "\n All the halfedges that go out from this vertex are: \n";
		for (VertexOutHalfedgeIterator vheit(ver); !vheit.end(); ++vheit) {
			Halfedge * he = *vheit;
			std::cout << " (" << he->source()->index() << ", " << he->target()->index() << ") ; ";
			std::cout << "halfedge index = " << he->index() << ".\n";
		}
		std::cout << "\n";
		break;
	}

	delete cMesh;
	system("pause");
	return 0;
}