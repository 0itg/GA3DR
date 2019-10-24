#include "Mesh.h"
#include "MatUtil.h"
#include "Material.h"

#include <SFML/Graphics.hpp>
#include "c3ga.h"
#include "c3ga_util.h"

#include <algorithm>
#include <mutex>
#include <execution>

using namespace c3ga;

extern bool DEBUG_COLOR;
extern bool DEBUG_WIREFRAME;
extern bool DEBUG_NOSURFACES;
extern bool DEBUG_NOTEXTURES;
extern bool DEBUG_TOGGLESMOOTHSHADING;


MtlLib Mesh::mtlLib;

Triangle::Triangle(Mesh* mesh, int a, int b, int c,
	int d, int e, int f, Material* mtl, int g, int h, int i) {
	parent = mesh;
	if (mtl != nullptr) mtlPtr = mtl;
	else mtlPtr = parent->mtlLib.mtl["default"];
	v[0] = a; v[1] = b; v[2] = c;
	t[0] = d; t[1] = e; t[2] = f;
	// 0 index refers to default texture coords (0,0), (1,0), (0,1),
	// to be initialized in mesh
	if (g != -1) {
		n[0] = g; n[1] = h; n[2] = i;
	}
	// If no normals given, calculate face normal
	// for flat shading based on vertices.
	else {
		plane P = _plane(no << parent->vertices[a]
			^ (no << parent->vertices[b]) ^ parent->vertices[c]);
		normalizePlane(P);
		int Pindex = parent->planes.size();
		parent->addPlane(P);
		n[0] = Pindex; n[1] = Pindex; n[2] = Pindex;
	}
}

// Moves mesh in model space, e.g. to adjust for inappropriate coordinates.
void Mesh::motion(c3ga::normalizedPoint location, c3ga::rotor R, float scale)
{
	float logScale = log(scale);
	normalizedTranslator T =
		_normalizedTranslator(1 - 0.5f * (location ^ noni) * ni);
	scalor S = _scalor(coshf(logScale/2) + sinhf(logScale/2)*noni);
	TRSversor TRS = _TRSversor(T * R * S);
	Mat4x4 placeMat = versorToFlatPointMatrix(TRS);
	Mat4x4 nMat = versorToPlaneMatrix(TRS);

	for (int v = 0; v < vertices.size(); ++v)
		vertices[v] = matrixFPMultiply(placeMat, vertices[v]);

	for (int n = 0; n < planes.size(); ++n)
		planes[n] = matrixPlaneMultiply(nMat, planes[n]);

	updateBoundingSphere();
}


void  Mesh::updateBoundingSphere()
{
	mv pld[3] = { (e1^no), (e2 ^ no), (e3 ^ no) };
	float minDist[3] = { 1e10f, 1e10f, 1e10f };
	float maxDist[3] = { -1e10f, -1e10f, -1e10f };
	for (int i = 0; i < vertices.size(); i++) {
		flatPoint p = vertices[i];
		for (int j = 0; j < 3; j++) {
			minDist[j] = std::min(_Float((pld[j] << p)), minDist[j]);
			maxDist[j] = std::max(_Float((pld[j] << p)), maxDist[j]);
		}
	}
	normalizedPoint center(c3gaPoint(0.5f * (minDist[0] + maxDist[0]), 0.5f *
				(minDist[1] + maxDist[1]), 0.5f * (minDist[2] + maxDist[2])));

	float r = 0;
	for (int i = 0; i < vertices.size(); i++)
	{
		flatPoint p = vertices[i];
		normalizedPoint v = _normalizedPoint(c3gaPoint(p.e1ni(),
											 p.e2ni(), p.e3ni()));
		r = std::min(_Float(v << center), r);
	}
	boundingSphere = center + r * ni;
	boundingSphereR = r;
}

void Mesh::recalcPlanes(bool smooth)
{
	planes.clear();

	for (Triangle& tri : faces) {
		// Flat shading: calculate one normal perpendicular
		// to the triangle, same for all three vertices. 
		plane P = _plane((no << tri.a()) ^ (no << tri.b()) ^ tri.c());
		normalizePlane(P);
		int Pindex = planes.size();
		addPlane(P);
		tri.n[0] = Pindex; tri.n[1] = Pindex; tri.n[2] = Pindex;
	}
	if (smooth == true) {
		std::vector<plane, boost::pool_allocator<plane,
			boost::default_user_allocator_malloc_free>> newPlane;
		newPlane.resize(planes.size());
		// For each triangle, pair the vertex indices with their
		// corresponding plane indices. Use the vertex indices to
		// add together all planes corresponding to the same index
		// and normalize. Every triangle sharing a vertex will now
		// use this averaged plane with that vertex.
		std::vector<std::pair<int, int>> vertexPlanePairs;
		for (const Triangle& tri : faces) {
			vertexPlanePairs.emplace_back(tri.v[0], tri.n[0]);
			vertexPlanePairs.emplace_back(tri.v[1], tri.n[1]);
			vertexPlanePairs.emplace_back(tri.v[2], tri.n[2]);
		}
		plane P = _plane(0);
		std::for_each(std::execution::par_unseq, vertexPlanePairs.begin(),
			vertexPlanePairs.end(), [&](auto& pair) {
				newPlane[pair.first] += planes[pair.second];
			});
		std::for_each(std::execution::par_unseq, faces.begin(),
			faces.end(), [](Triangle& tri) {
				tri.n[0] = tri.v[0];
				tri.n[1] = tri.v[1];
				tri.n[2] = tri.v[2];
			});
		std::for_each(std::execution::par_unseq, newPlane.begin(),
			newPlane.end(), [](plane& P) { normalizePlane(P); });
		planes.clear();
		planes = newPlane;
	}
}

// Adds new triangles to mesh created from clip as
// well as necessary vertices. Removes unused triangles

//std::mutex writingToFaces;
//void Mesh::clip() {
//	const dualPlane clipPlanes[4] = { _dualPlane(-e1 + e3), _dualPlane(e1 + e3),
//			_dualPlane(-e2 + e3),  _dualPlane(e2 + e3) };// , _dualPlane(e3 + clipDist * ni)};
//
//	for (const auto& N : clipPlanes)
//	{
//		// MEET of bounding sphere with clipping plane
//		circle X = _circle(dual(N ^ boundingSphere));
//		// If the square is positive, bounding sphere intersects the clipping plane
//		float circleSize = _Float(X * X);
//		float centerDist = dual(dual(boundingSphere ^ ni) ^ N).ni();
//		if (circleSize < 0 && centerDist > 0) continue;
//
//		size_t faceCount = faces.size();
//		std::vector<Triangle> newFaces;
//		std::for_each_n(std::execution::par_unseq, faces.begin(),
//			faceCount, [this, N, &newFaces](Triangle& tri) {
//
//			flatPoint a = tri.a();
//			flatPoint b = tri.b();
//			flatPoint c = tri.c();
//
//			// Normalize N.necessary for offset planes, but not
//			// with the planes currently used.
//			// float n = 1/sqrt(N.e1()*N.e1()+N.e2()*N.e2()+N.e3()*N.e3()); 
//			// N.m_c[0] *= n; N.m_c[1] *= n; N.m_c[2] *= n;
//
//			float aDistance = dual(dual(a) ^ N).ni();
//			float bDistance = dual(dual(b) ^ N).ni();
//			float cDistance = dual(dual(c) ^ N).ni();
//
//			int pointsOnScreen = 0;
//			if (aDistance > 0) ++pointsOnScreen;
//			if (bDistance > 0) ++pointsOnScreen;
//			if (cDistance > 0) ++pointsOnScreen;
//
//			switch (pointsOnScreen) {
//			case 0:
//				tri.v[0] = -1;
//				break;
//			case 1:
//			{
//				while (aDistance < 0) {
//					tri.cycle();
//					std::swap(aDistance, bDistance);
//					std::swap(aDistance, cDistance);
//				}
//				a = tri.a();
//				b = tri.b();
//				c = tri.c();
//				sf::Vector2f h = tri.ta();
//				sf::Vector2f i = tri.tb();
//				sf::Vector2f j = tri.tc();
//				plane k = tri.na();
//				plane l = tri.nb();
//				plane m = tri.nc();
//
//				float t = -(N.e1() * a.e1ni() + N.e2() * a.e2ni() + N.e3() * a.e3ni() - N.ni()) /
//					(N.e1() * (b.e1ni() - a.e1ni()) + N.e2() * (b.e2ni() - a.e2ni())
//						+ N.e3() * (b.e3ni() - a.e3ni()));
//				float s = 1 - t;
//
//				flatPoint D = linearCombo(a, s, b, t);;
//				sf::Vector2f F = h * s + i * t;
//				plane H = linearCombo(k, s, l, t);;
//
//				t = -(N.e1() * a.e1ni() + N.e2() * a.e2ni() + N.e3() * a.e3ni() - N.ni()) /
//					(N.e1() * (c.e1ni() - a.e1ni()) + N.e2() * (c.e2ni() - a.e2ni())
//						+ N.e3() * (c.e3ni() - a.e3ni()));
//				s = 1 - t;
//
//				flatPoint E = linearCombo(a, s, c, t);;
//				sf::Vector2f G = h * s + j * t;
//				plane I = linearCombo(k, s, m, t);;
//
//				// Keep the triangle, but change the offscreen vertices to the new ones. 
//
//				std::lock_guard<std::mutex> lock(writingToFaces);
//				int end = vertices.size();
//				int texEnd = texCoords.size();
//				int normEnd = planes.size();
//
//				addVertex(D);
//				addVertex(E);
//				tri.v[1] = end;
//				tri.v[2] = end + 1;
//
//				addTexCoord(F);
//				addTexCoord(G);
//				tri.t[1] = texEnd;
//				tri.t[2] = texEnd + 1;
//
//				addPlane(H);
//				addPlane(I);
//				tri.n[1] = normEnd;
//				tri.n[2] = normEnd + 1;
//
//				if (DEBUG_COLOR == true) tri.mtlPtr = mtlLib.mtl["red"];
//				break;
//			}
//			case 2:
//			{
//				while (aDistance < 0 || bDistance < 0) {
//					tri.cycle();
//					std::swap(aDistance, bDistance);
//					std::swap(aDistance, cDistance);
//				}
//				a = tri.a();
//				b = tri.b();
//				c = tri.c();
//				sf::Vector2f h = tri.ta();
//				sf::Vector2f i = tri.tb();
//				sf::Vector2f j = tri.tc();
//				plane k = tri.na();
//				plane l = tri.nb();
//				plane m = tri.nc();
//
//				float t = -(N.e1() * c.e1ni() + N.e2() * c.e2ni() + N.e3() * c.e3ni() - N.ni()) /
//					(N.e1() * (a.e1ni() - c.e1ni()) + N.e2() * (a.e2ni() - c.e2ni())
//						+ N.e3() * (a.e3ni() - c.e3ni()));
//				float s = 1 - t;
//
//				flatPoint D = linearCombo(c, s, a, t); // c*s + a*t
//				sf::Vector2f F = j * s + h * t;
//				plane H = linearCombo(m, s, k, t);
//
//				t = -(N.e1() * c.e1ni() + N.e2() * c.e2ni() + N.e3() * c.e3ni() - N.ni()) /
//					(N.e1() * (b.e1ni() - c.e1ni()) + N.e2() * (b.e2ni() - c.e2ni())
//						+ N.e3() * (b.e3ni() - c.e3ni()));
//				s = 1 - t;
//
//				flatPoint E = linearCombo(c, s, b, t);
//				sf::Vector2f G = j * s + i * t;
//				plane I = linearCombo(m, s, l, t);;
//
//				std::lock_guard<std::mutex> lock(writingToFaces);
//
//				int end = vertices.size();
//				int texEnd = texCoords.size();
//				int normEnd = planes.size();
//
//				// One triangle must be replaced by two.
//				// Change the original and add the other to the end.
//				addVertex(D);
//				addVertex(E);
//				tri.v[2] = end; // triangle ABD
//
//				addTexCoord(F);
//				addTexCoord(G);
//				tri.t[2] = texEnd;
//
//				addPlane(H);
//				addPlane(I);
//				tri.n[2] = normEnd;
//
//				newFaces.emplace_back(this, tri.v[1], end + 1, end,
//					tri.t[1], texEnd + 1, texEnd, tri.mtlPtr,
//					tri.n[1], normEnd + 1, normEnd);
//
//				if (DEBUG_COLOR == true) {
//					tri.mtlPtr = mtlLib.mtl["blue"];
//					newFaces.back().mtlPtr = mtlLib.mtl["green"];
//				}
//				break;
//			}
//			}
//		});
//		auto trisToRemove = std::remove_if(faces.begin(), faces.end(),
//			[](auto& tri) { return tri.v[0] == -1; });
//		faces.erase(trisToRemove, faces.end());
//		faces.insert(faces.end(), newFaces.begin(), newFaces.end());
//		newFaces.clear();
//	}
//}
void Mesh::clip() {
	const dualPlane clipPlanes[4] = { _dualPlane(-e1 + e3), _dualPlane(e1 + e3),
			_dualPlane(-e2 + e3),  _dualPlane(e2 + e3) };// , _dualPlane(e3 + clipDist * ni)};

	for (const dualPlane& N : clipPlanes)
	{
		int newTris = 0;
		int faceCount = faces.size();
		// MEET of bounding sphere with clipping plane
		circle X = _circle(dual(N ^ boundingSphere));
		// If the square is positive, bounding sphere intersects the clipping plane
		float circleSize = _Float(X * X);
		float centerDist = dual(dual(boundingSphere ^ ni) ^ N).ni();
		if (circleSize < 0 && centerDist > 0) continue;

		for (int f = 0; f < faceCount; f++) {
			Triangle& tri = faces[f];
			flatPoint&& a = tri.a();
			flatPoint&& b = tri.b();
			flatPoint&& c = tri.c();

			// Normalize N.necessary for offset planes, but not with the planes currently used.
			//float n = 1 / sqrt(N.e1()*N.e1() + N.e2()*N.e2() + N.e3()*N.e3()); 
			//N.m_c[0] *= n; N.m_c[1] *= n; N.m_c[2] *= n;

			float aDistance = dual(dual(a) ^ N).ni();
			float bDistance = dual(dual(b) ^ N).ni();
			float cDistance = dual(dual(c) ^ N).ni();

			int pointsOnScreen = 0;
			if (aDistance > 0) ++pointsOnScreen;
			if (bDistance > 0) ++pointsOnScreen;
			if (cDistance > 0) ++pointsOnScreen;

			switch (pointsOnScreen) {
			case 0:
				// Delete this triangle by swapping with the last one and popping
				tri = std::move(faces.back());
				faces.pop_back();
				// If this triangle hasn't been processed, decrement f
				//so we get it next iteration
				if (newTris == 0) { --f; --faceCount; }
				// But if any new triangles have been added to the end,
				//we don't want to process them
				else --newTris;
				break;
			case 1:
			{
				while (aDistance < 0) {
					tri.cycle();
					std::swap(aDistance, bDistance);
					std::swap(aDistance, cDistance);
				}
				a = tri.a();
				b = tri.b();
				c = tri.c();
				sf::Vector2f&& h = tri.ta();
				sf::Vector2f&& i = tri.tb();
				sf::Vector2f&& j = tri.tc();
				plane&& k = tri.na();
				plane&& l = tri.nb();
				plane&& m = tri.nc();

				// compute parameter for intersection with line
				// from a to b and clipping plane
				float num = -(N.e1() * a.e1ni() + N.e2() * a.e2ni()
					+ N.e3() * a.e3ni()/* - N.ni()*/);

				float t = num / (N.e1() * (b.e1ni() - a.e1ni()) +
					N.e2() * (b.e2ni() - a.e2ni()) + N.e3() * (b.e3ni() - a.e3ni()));
				float s = 1 - t;

				flatPoint&& D = linearCombo(a, s, b, t);;
				sf::Vector2f&& F = h * s + i * t;
				plane&& H = linearCombo(k, s, l, t);;

				// a to c
				t = num / (N.e1() * (c.e1ni() - a.e1ni())
					+ N.e2() * (c.e2ni() - a.e2ni()) + N.e3() * (c.e3ni() - a.e3ni()));
				s = 1 - t;

				flatPoint&& E = linearCombo(a, s, c, t);;
				sf::Vector2f&& G = h * s + j * t;
				plane&& I = linearCombo(k, s, m, t);;

				// Keep the triangle, but change the offscreen vertices to the new ones. 

				int end = vertices.size();
				int texEnd = texCoords.size();
				int normEnd = planes.size();

				addVertex(D);
				addVertex(E);
				tri.v[1] = end;
				tri.v[2] = end + 1;

				addTexCoord(F);
				addTexCoord(G);
				tri.t[1] = texEnd;
				tri.t[2] = texEnd + 1;

				addPlane(H);
				addPlane(I);
				tri.n[1] = normEnd;
				tri.n[2] = normEnd + 1;

				if (DEBUG_COLOR == true) tri.mtlPtr = mtlLib.mtl["red"];
				break;
			}
			case 2:
			{
				while (aDistance < 0 || bDistance < 0) {
					tri.cycle();
					std::swap(aDistance, bDistance);
					std::swap(aDistance, cDistance);
				}
				a = tri.a();
				b = tri.b();
				c = tri.c();
				sf::Vector2f&& h = tri.ta();
				sf::Vector2f&& i = tri.tb();
				sf::Vector2f&& j = tri.tc();
				plane&& k = tri.na();
				plane&& l = tri.nb();
				plane&& m = tri.nc();

				// compute parameter for intersection with line
				// from c to a and clipping plane
				float num = -(N.e1() * c.e1ni() + N.e2() * c.e2ni()
					+ N.e3() * c.e3ni()/* - N.ni()*/);
				float t = num / (N.e1() * (a.e1ni() - c.e1ni())
					+ N.e2() * (a.e2ni() - c.e2ni()) + N.e3() * (a.e3ni() - c.e3ni()));
				float s = 1 - t;

				flatPoint&& D = linearCombo(c, s, a, t); // c*s + a*t
				sf::Vector2f&& F = j * s + h * t;
				plane&& H = linearCombo(m, s, k, t);

				// c to b
				t = num / (N.e1() * (b.e1ni() - c.e1ni())
					+ N.e2() * (b.e2ni() - c.e2ni()) + N.e3() * (b.e3ni() - c.e3ni()));
				s = 1 - t;

				flatPoint&& E = linearCombo(c, s, b, t);
				sf::Vector2f&& G = j * s + i * t;
				plane&& I = linearCombo(m, s, l, t);;

				int end = vertices.size();
				int texEnd = texCoords.size();
				int normEnd = planes.size();

				// One triangle must be replaced by two.
				// Change the original and add the other to the end.
				addVertex(D);
				addVertex(E);
				tri.v[2] = end; // triangle ABD

				addTexCoord(F);
				addTexCoord(G);
				tri.t[2] = texEnd;

				addPlane(H);
				addPlane(I);
				tri.n[2] = normEnd;

				addTriangle(tri.v[1], end + 1, end,
					tri.t[1], texEnd + 1, texEnd, tri.mtlPtr,
					tri.n[1], normEnd + 1, normEnd);

				if (DEBUG_COLOR == true) {
					tri.mtlPtr = mtlLib.mtl["blue"];
					faces.back().mtlPtr = mtlLib.mtl["green"];
				}
				++newTris;
				break;
			}
			}
		}
	};
}