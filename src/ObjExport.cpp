#include "ObjExport.h"
#include "Pieces.h"
#include "Mat.h"



static float filterZero(float x) // don't allow numbers like 1.1e-16 to the output
{
	if (abs(x) < EPSILON)
		return 0.0f;
	return x;
}

void ObjExport::addMesh(const PicGroupDef *def, Mesh& mesh, const Mat4& fMatrix)
{
	bool didMtl = false;

	if (def != NULL && mtl != NULL) {
		if (def->drawtype == DRAW_COLOR)
		{
			(*mtl) << "newmtl material" << ++numObjs << "\n";
			(*mtl) << "  Ns 32\n  d 1\n  Tr 1\n  Tf 1 1 1\n  illum 2\n  Ka 0.0000 0.0000 0.0000\n  Ks 0.3500 0.3500 0.3500\n";
			(*mtl) << "  Kd " << def->r << " " << def->g << " " <<  def->b << "\n";
			didMtl = true;
		}
	}

	meshout << "g Object" << numObjs << "\n";

	if (didMtl) {
		meshout << "usemtl material" << numObjs << "\n";
	} 

	bool hasTex = false;

	// We found unique vertices. Now let's put them to file
	for (int i = 0; i < mesh.m_vtx.size(); ++i) {
		meshout << "v " << filterZero(mesh.m_vtx[i].x) << " " << filterZero(mesh.m_vtx[i].y) << " " << filterZero(mesh.m_vtx[i].z) << "\n";
	}

// 	if (hasTex)	{
// 		for (int i = 0; i < realNumTexVers; ++i) {
// 			meshout << "vt " << texVertices[i].x << " " << texVertices[i].y << "\n";
// 		}
// 	}

	int elemSize = mesh.elemSize();
	for(int idxi = 0; idxi < mesh.m_idx.size(); idxi += elemSize) 
	{
		if (elemSize == 4) {
			int a = mesh.m_idx[idxi + 0] + numVerts,
				b = mesh.m_idx[idxi + 1] + numVerts,
				c = mesh.m_idx[idxi + 2] + numVerts,
				d = mesh.m_idx[idxi + 3] + numVerts;

			if (asTriangles) {
				meshout << "f " << a << " " << b << " " << c << "\n";
				meshout << "f " << a << " " << c << " " << d << "\n";
			}
			else {
				meshout << "f " << a << " " << b << " " << c << " " << d << "\n";
			}
		}
		else if (elemSize == 3) {
			meshout << "f " << mesh.m_idx[idxi + 0] + numVerts << " " <<
				               mesh.m_idx[idxi + 1] + numVerts << " " <<
				               mesh.m_idx[idxi + 2] + numVerts << "\n";
		}
	}

	numVerts += mesh.m_vtx.size();
	//numTexVerts += realNumTexVers;

}
