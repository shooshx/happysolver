// Happy Cube Solver - Building and designing models for the Happy cube puzzles
// Copyright (C) 1995-2006 Shy Shalom, shooshX@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "MyObject.h"
#include "Mesh.h"
#include <unordered_map>
using namespace std;


int MyPoint::g_ctorCount = 0, MyPoint::g_dtorCount = 0;
int MyPolygon::g_ctorCount = 0, MyPolygon::g_dtorCount = 0;

void MyObject::MyPointWrapper::detach(MyAllocator *m_alloc) const
{ 
    MyPoint *old = ptr;
    if (m_alloc != NULL)
        ptr = m_alloc->m_pointsPool.allocate();
    else
        ptr = new MyPoint; // TBD - happens with lines. remove when lines go display list.
    *ptr = *old;
}

// checks if a point is in m_tmppoints, if not, copy and insert it
// returns the pointer to the permanent point
MyPoint* MyObject::CopyCheckPoint(Vec3 *c)
{
    static MyPoint p;
    p.setp(*c);
    const MyPointWrapper &realpntw = *m_tmppoints.insert(MyPointWrapper(&p)).first;
    if (realpntw.ptr == &p) // it was inserted
        realpntw.detach(m_alloc);
    return realpntw.ptr;
}

void MyObject::addLine(Vec3 *inp1, Vec3 *inp2, double inR, double inG, double inB, ELineType type)
{	
    MyLine pln(NULL, NULL, inR, inG, inB, type);

    pln.p1 = CopyCheckPoint(inp1);
    pln.p2 = CopyCheckPoint(inp2);

    lnlst.push_back(pln);
}

// copies the points in the points array
void MyObject::addPoly(Vec3 *inplst, TexAnchor *ancs, Texture *tex)
{
    MyPolygon *nply = m_alloc->m_polyPool.allocate();
    nply->init(ancs, tex);
    for (int lstp = 0; lstp < 4; ++lstp)
    {
        nply->vtx[lstp] = CopyCheckPoint(&inplst[lstp]);
    }

    plylst.push_back(nply);
}

inline void MyObject::basicAddPoint(MyPoint *pnt) 
{ 
    pntlst.push_back(pnt); 
}

// add a polygon made of points in the repository, without duplicacy checks.
inline void MyObject::basicAddPoly(MyPoint *inparr[], TexAnchor *ancs)
{ 
    MyPolygon *nply = m_alloc->m_polyPool.allocate();
    nply->init(inparr, ancs);
    plylst.push_back(nply); 
}

bool operator==(const MyObject::MyPointWrapper &p1, const MyObject::MyPointWrapper &p2)
{
    return (*p1.ptr == *p2.ptr); // compare the actual values TBD..
}


void MyObject::vectorify()
{
    int insPoly = 0, insLines = 0, insPoint = 0;
    // points
    nPoints = m_tmppoints.size() + pntlst.size();
    
    if (points != NULL)
        delete[] points; //don't delete the points themselves.
    points = new MyPoint*[nPoints];

    for (auto pit = m_tmppoints.begin(); pit != m_tmppoints.end(); ++pit) {
        points[insPoint++] = const_cast<MyPoint*>(pit->ptr); // const_iterator is too restrictive
    }
    m_tmppoints.clear();
    for (auto plit = pntlst.cbegin(); plit != pntlst.cend(); ++plit)
    {
        points[insPoint++] = *plit;
    }
    pntlst.clear();

    if (poly != NULL)
        delete[] poly;
    nPolys = plylst.size();
    poly = new MyPolygon*[nPolys];

    // add the polygons sorted according to their texture, NULL first
    /*
    Texture *availTexs[4] = { NULL }; // no more the 4 textues in an object.. actually, no more then 3 in reality.
    int availTexAdd = 1, availTexGet = 0;
    while (insPoly < nPolys) // until we got all the polygons in
    {
        M_ASSERT(availTexGet < availTexAdd);
        for (TPolyList::iterator lit = plylst.begin(); lit != plylst.end() ; ++lit)
        {
            if ((*lit)->tex == availTexs[availTexGet])
                poly[insPoly++] = *lit;
            else if (((*lit)->tex != NULL) && ((*lit)->tex != availTexs[1]) && ((*lit)->tex != availTexs[2]) && ((*lit)->tex != availTexs[3]))
                availTexs[availTexAdd++] = (*lit)->tex;
        }
        ++availTexGet;
    }
    */
    plylst.clear();

    if (lines != NULL)
        delete[] lines;
    nLines = lnlst.size();
    lines = new MyLine[nLines];
    for (auto nit = lnlst.begin(); nit != lnlst.end(); ++nit)
    {
        lines[insLines++] = *nit;
    }
    lnlst.clear();

}

void MyObject::clacNormals(bool vtxNorms)
{
    verterxNormals = vtxNorms;
    int pn;
    for (pn = 0; pn < nPoints; ++pn)
    {
        points[pn]->n.clear();
    }

    for (int i = 0; i < nPolys; ++i)
    {
        poly[i]->calcNorm();
        if (vtxNorms)
        {
            for(pn = 0; pn < 4; ++pn)
            {
                poly[i]->vtx[pn]->n += poly[i]->center;
            }
        }
    }

    if (vtxNorms)
    {
        for (pn = 0; pn < nPoints; ++pn)
        {
            points[pn]->n.unitize();
        }
    }

}

/// PointPair is a simple struct of two MyPoints objects.
/// it is used for hashing HalfEdge objects in MyObject::buildHalfEdges()
struct PointPair
{
    PointPair(MyPoint *_p1, MyPoint *_p2) :p1(_p1), p2(_p2) {}
    // p1 and p2 are interchangable
    MyPoint *p1;
    MyPoint *p2;
};

struct PpHash {
    size_t operator()(const PointPair& pp) const
    {
        uint x = (pp.p1->hash() + pp.p2->hash()); // commutative
    //	qDebug("%f,%f,%f:%f,%f,%f - %X", (*pp.p1).p[0], (*pp.p1).p[1], (*pp.p1).p[2], (*pp.p2).p[0], (*pp.p2).p[1], (*pp.p2).p[2], x);
        return x;
    }
};

bool operator==(const PointPair& pp1, const PointPair& pp2)
{	// compare pointers
    return ((pp1.p1 == pp2.p1) && (pp1.p2 == pp2.p2)) || ((pp1.p2 == pp2.p1) && (pp1.p1 == pp2.p2)); // commutative
}
typedef unordered_map<PointPair, HalfEdge*, PpHash> TPointPairHash;


bool MyObject::buildHalfEdges(THalfEdgeList& lst)
{
    int pli, i;

    TPointPairHash m_pntmap;
    m_pntmap.reserve(150);
    // the pointer is the half edge already between these points

    for(pli = 0; pli < nPolys; ++pli)
    {
        MyPolygon *pol = poly[pli];

        HalfEdge *he = NULL, *lasthe = NULL;
        for(i = 3; i >= 0; --i)
        {
            //he = new HalfEdge(pol, pol->vtx[i], he);
            HalfEdge *prevhe = he;
            he = m_alloc->m_hePool.allocate();
            he->init(pol, pol->vtx[i], prevhe);
            lst.push_back(he);
            if (lasthe == NULL)
                lasthe = he;

            if (pol->vtx[i]->he == NULL)
                pol->vtx[i]->he = he;

        }
        lasthe->next = he;
        pol->he = he;

        for(i = 3; i >= 0; --i)
        {	
            PointPair pp(he->point, he->next->point);
            TPointPairHash::iterator it = m_pntmap.find(pp);
            if (it == m_pntmap.end()) // its not there, add it
                m_pntmap.insert(make_pair(pp, he));
            else
            {
                HalfEdge *she = it->second;
                M_ASSERT((she->next->point == he->point) && (he->next->point == she->point));
                he->pair = she;
                she->pair = he;
                m_pntmap.erase(it); // no longer needed in the map
            }
            he = he->next;
        }
        // maximum size of m_pntmap is 134
    }

    M_ASSERT(m_pntmap.empty());

    return true;
}


void MyObject::subdivide(bool smooth)
{
    int pli, i;

    m_alloc->m_hePool.clear();
    THalfEdgeList helst;
    helst.reserve(5000);
    buildHalfEdges(helst);

    // face points
    for(pli = 0; pli < nPolys; ++pli)
    {
        MyPolygon &pol = *poly[pli];
        pol.f = m_alloc->m_pointsPool.allocate();
        pol.f->clear();
        for(i = 0; i < 4; ++i)
        {
            *pol.f += *pol.vtx[i];
            pol.vtx[i]->touched = false; // while we're at it, reset the point for the current iteration
        }
        *pol.f /= 4;
        basicAddPoint(pol.f);
    }
    
    // edge points
    for(i = 0; i < helst.size(); ++i)
    {
        HalfEdge *he = helst[i];
        if (he->edgePoint != NULL) 
            continue;

        he->edgePoint = m_alloc->m_pointsPool.allocate();
        (*he->edgePoint) = *he->point;
        (*he->edgePoint) += *he->pair->point;
        if (smooth)
        {
            (*he->edgePoint) += *he->poly->f;
            (*he->edgePoint) += *he->pair->poly->f;
            (*he->edgePoint) /= 4.0;
        }
        else
            (*he->edgePoint) /= 2.0;

        he->pair->edgePoint = he->edgePoint;
        basicAddPoint(he->edgePoint);
    }

    for(int pli = 0; pli < nPolys; ++pli)
    {
        MyPolygon &pol = *poly[pli];
        MyPoint *e[4];

        i = 0;
        HalfEdge *he = pol.he;
        do {
            e[i++] = he->edgePoint;
            he = he->next;
        } while (he != pol.he);

        
        if (smooth)
        {
            for(i = 0; i < 4; ++i)
            {
                if (pol.vtx[i]->touched)
                    continue;

                Vec3 F, E; // average of all Face points near of op[i]

                double n = 0;
                he = pol.vtx[i]->he;
                do {
                    F += he->poly->f->p;
                    if (he->point == pol.vtx[i]) // pointer comparison
                        E += he->pair->point->p;
                    else
                        E += he->point->p;

                    he = he->pair->next;
                    n += 1.0;
                } while (he != pol.vtx[i]->he);

            /*	F /= n;
                E += *poly.vtx[i] * 3.0;
                E /= n * 2.0;
                op[i] = (F + (2.0 * E) + ((n - 3.0)*(*poly.vtx[i])))/n;*/

                // store the result intermediatly in 'n' so we wont affect the on-going calculation process
                double nsqr = n*n;
                pol.vtx[i]->n = (((n - 2.0)/n)*(pol.vtx[i]->p)) + (F/(nsqr)) + (E/(nsqr));

                pol.vtx[i]->he = NULL; // reset the half edge for the next time (and for the halfedge impending death)
                pol.vtx[i]->touched = true;
                basicAddPoint(pol.vtx[i]); // add it again becuse the array is going to be deleted
            }
        }
        else
        {
            for(i = 0; i < 4; ++i)
            {
                pol.vtx[i]->he = NULL; // reset the half edge for the next time (and for the halfedge impending death)
                pol.vtx[i]->touched = true;
                basicAddPoint(pol.vtx[i]); // add it again becuse the array is going to be deleted
            }
        }

        pol.he = NULL;		

        MyPoint *pl1[] = { pol.vtx[0], e[0], pol.f, e[3] };
        MyPoint *pl2[] = { e[0], pol.vtx[1], e[1], pol.f };
        MyPoint *pl3[] = { pol.f, e[1], pol.vtx[2], e[2] };
        MyPoint *pl4[] = { e[3], pol.f, e[2], pol.vtx[3] };


        TexAnchor tv[] = { pol.texAncs[0], pol.texAncs[1], pol.texAncs[2], pol.texAncs[3] }; // original texture anchors
        TexAnchor tf((tv[0].x + tv[2].x) / 2.0, (tv[0].y + tv[2].y) / 2.0);
        TexAnchor te[] = { TexAnchor((tv[0].x + tv[1].x) / 2.0, (tv[0].y + tv[1].y) / 2.0), 
                           TexAnchor((tv[1].x + tv[2].x) / 2.0, (tv[1].y + tv[2].y) / 2.0),
                           TexAnchor((tv[2].x + tv[3].x) / 2.0, (tv[2].y + tv[3].y) / 2.0),
                           TexAnchor((tv[3].x + tv[0].x) / 2.0, (tv[3].y + tv[0].y) / 2.0) };

        TexAnchor tx1[] = { tv[0], te[0], tf, te[3] };
        TexAnchor tx2[] = { te[0], tv[1], te[1], tf };
        TexAnchor tx3[] = { tf, te[1], tv[2], te[2] };
        TexAnchor tx4[] = { te[3], tf, te[2], tv[3] };

        basicAddPoly(pl1, tx1);
        basicAddPoly(pl2, tx2);
        basicAddPoly(pl3, tx3);
        basicAddPoly(pl4, tx4);

    }

    if (smooth)
    {
        // go over all the points the the points array of the object. these are the original points
        for (i = 0; i < nPoints; ++i)
            points[i]->p = points[i]->n;
    }

    vectorify();
}



void MyObject::toMesh(Mesh& mesh) 
{
    mesh.clear();
    VecRep vtxrep(&mesh.m_vtx);

    for(int pli = 0; pli < nPolys; ++pli) //polygons
    {
        MyPolygon &curpl = *poly[pli];
        for(int pni = 0; pni < 4; ++pni) //points
        {
            MyPoint *curpn = curpl.vtx[pni];
            int index = 0;
            if (vtxrep.add(curpn->p, &index)) {
                mesh.m_normals.push_back(curpn->n);
                //mesh.m_texCoord.push_back(Vec2(curpl.texAncs[pni].x, curpl.texAncs[pni].y));
            }
            mesh.m_idx.push_back(index);
        }

    }

    mesh.m_type = Mesh::QUADS;
    mesh.m_hasNormals = true;
//	mesh.m_hasTexCoord = true;
    mesh.m_hasIdx = true;

}