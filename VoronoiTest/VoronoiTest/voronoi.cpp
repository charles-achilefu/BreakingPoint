#include "voronoi.h"
#include <iostream>
#include <sstream>
using namespace voro;

double rnd() { return double(rand()) / RAND_MAX; }

void Voronoi::createVoronoiFile(int numSeeds, std::vector<float> offset, float scale, std::string outputFileName) {
	int i;
	double x, y, z;
	double x_min = -scale + offset[0], x_max = scale + offset[0];
	double y_min = -scale + offset[1], y_max = scale + offset[1];
	double z_min = -scale + offset[2], z_max = scale + offset[2];
	int n_x = 6, n_y = 6, n_z = 6;

	// Create a container with the geometry given above, and make it
	// non-periodic in each of the three coordinates. Allocate space for
	// eight particles within each computational block
	container con(x_min, x_max, y_min, y_max, z_min, z_max, n_x, n_y, n_z,
		false, false, false, 8);

	// Randomly add particles into the container
	float f = 0.2f;
	for (i = 0; i < 4; i++) {
		x = offset[0] + (f * scale * (-1 + (2 * rnd())));
		y = offset[1] + (f * scale * (-1 + (2 * rnd())));
		z = offset[2] + (f * scale * (-1 + (2 * rnd())));
		con.put(i, x, y, z);
	}
	for (i = 4; i<numSeeds; i++) {
		x = x_min + rnd()*(x_max - x_min);
		y = y_min + rnd()*(y_max - y_min);
		z = z_min + rnd()*(z_max - z_min);
		con.put(i, x, y, z);
	}

	con.print_custom("%w\n%P\n%s\n%t", outputFileName.c_str());
}

void Voronoi::testWeb(int numSeeds, std::vector<float> offset, float scale, std::string outputFileName) {
	int i;
	double x, y, z;
	double x_min = -scale + offset[0], x_max = scale + offset[0];
	double y_min = -scale + offset[1], y_max = scale + offset[1];
	double z_min = -scale + offset[2], z_max = scale + offset[2];
	int n_x = 6, n_y = 6, n_z = 6;

	container con(x_min, x_max, y_min, y_max, z_min, z_max, n_x, n_y, n_z, false, false, false, 8);

	/*for (i = 0; i < numSeeds - 10; i++) {
		float r = rnd() * (x_max - x_min) / 5.0;
		float theta = rnd() * 360.0;
		x = r * cos(theta);
		y = r * sin(theta);
		z = 0.0;
		con.put(i, x, y, z);
	}
	for (i = numSeeds - 10; i < numSeeds; i++) {
		float r = rnd() * (x_max - x_min) / 2.0;
		float theta = rnd() * 360.0;
		x = r * cos(theta);
		y = r * sin(theta);
		z = 0.0;
		con.put(i, x, y, z);
	}*/
	float r = 0.03125f;
	for (i = 0; i < 4; i++) {
		r = r * 2.0f;
		for (int j = 0; j < 20; j++) {
			float theta = 30.0f * j + rnd();
			x = r * cos(theta);
			y = r * sin(theta);
			z = 0.0;
			con.put(j + i*12, x, y, z);
		}
	}

	con.print_custom("%w\n%P\n%s\n%t", "web.txt");
}

void Voronoi::triangulateFaces(Faces &trifaceData, const Faces &faceData) {
	for (int i = 0; i < faceData.size(); i++) {
		for (int j = 0; j < faceData[i].size() - 2; j++) {
			std::vector<int> tri = { faceData[i][0], faceData[i][j + 2],faceData[i][j + 1] };
			trifaceData.push_back(tri);
		}
	}
}

std::vector<Geometry> Voronoi::parseVoronoi(std::string voronoiFile) {
	std::vector<Geometry> voroData = std::vector<Geometry>();

	FILE *voroFile = fopen(voronoiFile.c_str(), "r");
	if (voroFile == NULL) {
		printf("IOError: %s couldn't be opened.\n", voronoiFile.c_str());
		return voroData;
	}

	char line[1000];

	fgets(line, 1000, voroFile);
	while (true) {
		// Get number of vertices
		int numVertices;
		sscanf(line, "%i", &numVertices);

		// Get vertices
		Points vertexData = Points();
		fgets(line, 1000, voroFile);
		int numChars = 0;
		for (int i = 0; i < numVertices; i++) {
			double x, y, z;
			int numCharsRead;
			sscanf(line + numChars, "(%lf,%lf,%lf) %n", &x, &y, &z, &numCharsRead);
			numChars += numCharsRead;

			std::vector<double> vertex = std::vector<double>();
			vertex.push_back(x);
			vertex.push_back(y);
			vertex.push_back(z);

			vertexData.push_back(vertex);
		}

		// Get number of faces
		int numFaces;
		fgets(line, 1000, voroFile);
		sscanf(line, "%i", &numFaces);

		// Get face indices
		Faces faceData = Faces();
		fgets(line, 1000, voroFile);
		numChars = 0;
		for (int i = 0; i < numFaces; i++) {
			// Get face data
			char face[1000];
			int numCharsRead;
			sscanf(line + numChars, "%s %n", &face, &numCharsRead);
			numChars += numCharsRead;

			// Save indices
			std::vector<int> vertexIndices = std::vector<int>();
			std::string str(face);
			str = str.substr(1, str.length() - 2);
			std::stringstream ss(str);
			std::string token;
			while (std::getline(ss, token, ',')) {
				int f = std::stoi(token);
				vertexIndices.push_back(f);
			}

			faceData.push_back(vertexIndices);
		}
		Faces trifaceData;
		triangulateFaces(trifaceData, faceData);
		Geometry pairData = Geometry(vertexData, trifaceData);
		voroData.push_back(pairData);

		// Termination
		if (!fgets(line, 1000, voroFile)) {
			break;
		}
	}
	fclose(voroFile);

	return voroData;
}