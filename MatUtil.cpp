#include "MatUtil.h"
#include "c3ga.h"

using namespace c3ga;

Mat4x4 matrixMultiply(const Mat4x4& A, const Mat4x4& B) {
	return Mat4x4{ A.c[0 + 0 * 4] * B.c[0 + 0 * 4] + A.c[1 + 0 * 4] * B.c[0 + 1 * 4] + A.c[2 + 0 * 4] * B.c[0 + 2 * 4] + A.c[3 + 0 * 4] * B.c[0 + 3 * 4],
				   A.c[0 + 0 * 4] * B.c[1 + 0 * 4] + A.c[1 + 0 * 4] * B.c[1 + 1 * 4] + A.c[2 + 0 * 4] * B.c[1 + 2 * 4] + A.c[3 + 0 * 4] * B.c[1 + 3 * 4],
				   A.c[0 + 0 * 4] * B.c[2 + 0 * 4] + A.c[1 + 0 * 4] * B.c[2 + 1 * 4] + A.c[2 + 0 * 4] * B.c[2 + 2 * 4] + A.c[3 + 0 * 4] * B.c[2 + 3 * 4],
				   A.c[0 + 0 * 4] * B.c[3 + 0 * 4] + A.c[1 + 0 * 4] * B.c[3 + 1 * 4] + A.c[2 + 0 * 4] * B.c[3 + 2 * 4] + A.c[3 + 0 * 4] * B.c[3 + 3 * 4],
				   A.c[0 + 1 * 4] * B.c[0 + 0 * 4] + A.c[1 + 1 * 4] * B.c[0 + 1 * 4] + A.c[2 + 1 * 4] * B.c[0 + 2 * 4] + A.c[3 + 1 * 4] * B.c[0 + 3 * 4],
				   A.c[0 + 1 * 4] * B.c[1 + 0 * 4] + A.c[1 + 1 * 4] * B.c[1 + 1 * 4] + A.c[2 + 1 * 4] * B.c[1 + 2 * 4] + A.c[3 + 1 * 4] * B.c[1 + 3 * 4],
				   A.c[0 + 1 * 4] * B.c[2 + 0 * 4] + A.c[1 + 1 * 4] * B.c[2 + 1 * 4] + A.c[2 + 1 * 4] * B.c[2 + 2 * 4] + A.c[3 + 1 * 4] * B.c[2 + 3 * 4],
				   A.c[0 + 1 * 4] * B.c[3 + 0 * 4] + A.c[1 + 1 * 4] * B.c[3 + 1 * 4] + A.c[2 + 1 * 4] * B.c[3 + 2 * 4] + A.c[3 + 1 * 4] * B.c[3 + 3 * 4],
				   A.c[0 + 2 * 4] * B.c[0 + 0 * 4] + A.c[1 + 2 * 4] * B.c[0 + 1 * 4] + A.c[2 + 2 * 4] * B.c[0 + 2 * 4] + A.c[3 + 2 * 4] * B.c[0 + 3 * 4],
				   A.c[0 + 2 * 4] * B.c[1 + 0 * 4] + A.c[1 + 2 * 4] * B.c[1 + 1 * 4] + A.c[2 + 2 * 4] * B.c[1 + 2 * 4] + A.c[3 + 2 * 4] * B.c[1 + 3 * 4],
				   A.c[0 + 2 * 4] * B.c[2 + 0 * 4] + A.c[1 + 2 * 4] * B.c[2 + 1 * 4] + A.c[2 + 2 * 4] * B.c[2 + 2 * 4] + A.c[3 + 2 * 4] * B.c[2 + 3 * 4],
				   A.c[0 + 2 * 4] * B.c[3 + 0 * 4] + A.c[1 + 2 * 4] * B.c[3 + 1 * 4] + A.c[2 + 2 * 4] * B.c[3 + 2 * 4] + A.c[3 + 2 * 4] * B.c[3 + 3 * 4],
				   A.c[0 + 3 * 4] * B.c[0 + 0 * 4] + A.c[1 + 3 * 4] * B.c[0 + 1 * 4] + A.c[2 + 3 * 4] * B.c[0 + 2 * 4] + A.c[3 + 3 * 4] * B.c[0 + 3 * 4],
				   A.c[0 + 3 * 4] * B.c[1 + 0 * 4] + A.c[1 + 3 * 4] * B.c[1 + 1 * 4] + A.c[2 + 3 * 4] * B.c[1 + 2 * 4] + A.c[3 + 3 * 4] * B.c[1 + 3 * 4],
				   A.c[0 + 3 * 4] * B.c[2 + 0 * 4] + A.c[1 + 3 * 4] * B.c[2 + 1 * 4] + A.c[2 + 3 * 4] * B.c[2 + 2 * 4] + A.c[3 + 3 * 4] * B.c[2 + 3 * 4],
				   A.c[0 + 3 * 4] * B.c[3 + 0 * 4] + A.c[1 + 3 * 4] * B.c[3 + 1 * 4] + A.c[2 + 3 * 4] * B.c[3 + 2 * 4] + A.c[3 + 3 * 4] * B.c[3 + 3 * 4], };
}

void matrixTranspose(Mat4x4& A) {
	for (int i = 1; i < 4; i++)
		for (int j = 0; j < i; j++)
			if (i != j) {
				float temp = A.c[j + 4 * i];
				A.c[j + 4 * i] = A.c[i + 4 * j];
				A.c[i + 4 * j] = temp;
			}
}

Mat4x4 versorToFlatPointMatrix(const TRversor& R) {
	TRversor RI = inverse(R);
	flatPoint a = _flatPoint(R * e1ni * RI);
	flatPoint b = _flatPoint(R * e2ni * RI);
	flatPoint c = _flatPoint(R * e3ni * RI);
	flatPoint d = _flatPoint(R * noni * RI);
	return Mat4x4{a.e1ni(), b.e1ni(), c.e1ni(), d.e1ni(),
				  a.e2ni(), b.e2ni(), c.e2ni(), d.e2ni(),
				  a.e3ni(), b.e3ni(), c.e3ni(), d.e3ni(),
				  a.noni(), b.noni(), c.noni(), d.noni()};
}

Mat4x4 versorToFlatPointMatrix(const TRSversor& R) {
	TRSversor RI = inverse(R);
	flatPoint a = _flatPoint(R * e1ni * RI);
	flatPoint b = _flatPoint(R * e2ni * RI);
	flatPoint c = _flatPoint(R * e3ni * RI);
	flatPoint d = _flatPoint(R * noni * RI);
	return Mat4x4{ a.e1ni(), b.e1ni(), c.e1ni(), d.e1ni(),
				  a.e2ni(), b.e2ni(), c.e2ni(), d.e2ni(),
				  a.e3ni(), b.e3ni(), c.e3ni(), d.e3ni(),
				  a.noni(), b.noni(), c.noni(), d.noni() };
}

Mat4x4 versorToDualPlaneMatrix(const TRversor& R) {
	TRversor RI = inverse(R);
	dualPlane a = _dualPlane(R * e1 * RI);
	dualPlane b = _dualPlane(R * e2 * RI);
	dualPlane c = _dualPlane(R * e3 * RI);
	dualPlane d = _dualPlane(R * no * RI);
	return Mat4x4{a.e1(), b.e1(), c.e1(), d.e1(),
				  a.e2(), b.e2(), c.e2(), d.e2(),
				  a.e3(), b.e3(), c.e3(), d.e3(), 
				  a.ni(), b.ni(), c.ni(), d.ni()};
}

Mat4x4 versorToDualPlaneMatrix(const TRSversor& R) {
	TRSversor RI = inverse(R);
	dualPlane a = _dualPlane(R * e1 * RI);
	dualPlane b = _dualPlane(R * e2 * RI);
	dualPlane c = _dualPlane(R * e3 * RI);
	dualPlane d = _dualPlane(R * no * RI);
	return Mat4x4{ a.e1(), b.e1(), c.e1(), d.e1(),
				  a.e2(), b.e2(), c.e2(), d.e2(),
				  a.e3(), b.e3(), c.e3(), d.e3(),
				  a.ni(), b.ni(), c.ni(), d.ni() };
}

Mat4x4 versorToPlaneMatrix(const TRversor& R) {
	TRversor RI = inverse(R);
	plane a = _plane(R * (e1^e2^e3^ni) * RI);
	plane b = _plane(R * (e1 ^ e2 ^ no ^ ni) * RI);
	plane c = _plane(R * (e1 ^ e3 ^ no ^ ni) * RI);
	plane d = _plane(R * (e2 ^ e3 ^ no ^ ni) * RI);
	return Mat4x4{a.e1e2e3ni(), b.e1e2e3ni(), c.e1e2e3ni(), d.e1e2e3ni(),
				  a.e1e2noni(), b.e1e2noni(), c.e1e2noni(), d.e1e2noni(),
				  a.e1e3noni(), b.e1e3noni(), c.e1e3noni(), d.e1e3noni(),
				  a.e2e3noni(), b.e2e3noni(), c.e2e3noni(), d.e2e3noni()};
}

Mat4x4 versorToPlaneMatrix(const TRSversor& R) {
	TRSversor RI = inverse(R);
	plane a = _plane(R * (e1 ^ e2 ^ e3 ^ ni) * RI);
	plane b = _plane(R * (e1 ^ e2 ^ no ^ ni) * RI);
	plane c = _plane(R * (e1 ^ e3 ^ no ^ ni) * RI);
	plane d = _plane(R * (e2 ^ e3 ^ no ^ ni) * RI);
	return Mat4x4{ a.e1e2e3ni(), b.e1e2e3ni(), c.e1e2e3ni(), d.e1e2e3ni(),
				  a.e1e2noni(), b.e1e2noni(), c.e1e2noni(), d.e1e2noni(),
				  a.e1e3noni(), b.e1e3noni(), c.e1e3noni(), d.e1e3noni(),
				  a.e2e3noni(), b.e2e3noni(), c.e2e3noni(), d.e2e3noni() };
}

Mat5x5 versorToPointMatrix(const TRversor& R) {
	TRversor RI = inverse(R);
	point o = _point(R * no * RI);
	point a = _point(R * e1 * RI);
	point b = _point(R * e2 * RI);
	point c = _point(R * e3 * RI);
	point d = _point(R * no * RI);
	return Mat5x5{o.no(), a.no(), b.no(), c.no(), d.no(),
				  o.e1(), a.e1(), b.e1(), c.e1(), d.e1(),
				  o.e2(), a.e2(), b.e2(), c.e2(), d.e2(),
				  o.e3(), a.e3(), b.e3(), c.e3(), d.e3(),
				  o.ni(), a.ni(), b.ni(), c.ni(), d.ni()};
}