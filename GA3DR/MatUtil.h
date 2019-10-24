#pragma once
#include "c3ga.h"
#include "c3ga_util.h"

struct Mat4x4 {
	float c[16];
};
struct Mat5x5 {
	float c[25];
};

Mat4x4 matrixMultiply(const Mat4x4& A, const Mat4x4& B);
void matrixTranspose(Mat4x4& A);
Mat4x4 versorToFlatPointMatrix(const c3ga::TRversor& R);
Mat4x4 versorToDualPlaneMatrix(const c3ga::TRversor& R);
Mat4x4 versorToPlaneMatrix(const c3ga::TRversor& R);
Mat5x5 versorToPointMatrix(const c3ga::TRversor& R);
Mat4x4 versorToFlatPointMatrix(const c3ga::TRSversor& R);
Mat4x4 versorToDualPlaneMatrix(const c3ga::TRSversor& R);
Mat4x4 versorToPlaneMatrix(const c3ga::TRSversor& R);
Mat5x5 versorToPointMatrix(const c3ga::TRSversor& R);
Mat4x4 omPlane(const Mat5x5& M);

inline c3ga::plane matrixPlaneMultiply(const Mat4x4& A, const c3ga::plane& B) {
	return c3ga::plane(c3ga::plane_e1e2e3ni_e1e2noni_e1e3noni_e2e3noni,
		A.c[0 + 0 * 4] * B.e1e2e3ni() + A.c[1 + 0 * 4] * B.e1e2noni() + A.c[2 + 0 * 4] * B.e1e3noni() + A.c[3 + 0 * 4] * B.e2e3noni(),
		A.c[0 + 1 * 4] * B.e1e2e3ni() + A.c[1 + 1 * 4] * B.e1e2noni() + A.c[2 + 1 * 4] * B.e1e3noni() + A.c[3 + 1 * 4] * B.e2e3noni(),
		A.c[0 + 2 * 4] * B.e1e2e3ni() + A.c[1 + 2 * 4] * B.e1e2noni() + A.c[2 + 2 * 4] * B.e1e3noni() + A.c[3 + 2 * 4] * B.e2e3noni(),
		A.c[0 + 3 * 4] * B.e1e2e3ni() + A.c[1 + 3 * 4] * B.e1e2noni() + A.c[2 + 3 * 4] * B.e1e3noni() + A.c[3 + 3 * 4] * B.e2e3noni());
}
inline c3ga::point matrixPointMultiply(const Mat5x5& A, const c3ga::point& B) {
	return c3ga::point(c3ga::point_no_e1_e2_e3_ni,
		A.c[0 + 0 * 5] * B.no() + A.c[1 + 0 * 5] * B.e1() + A.c[2 + 0 * 5] * B.e2() + A.c[3 + 0 * 5] * B.e3() + A.c[4 + 0 * 5] * B.ni(),
		A.c[0 + 1 * 5] * B.no() + A.c[1 + 1 * 5] * B.e1() + A.c[2 + 1 * 5] * B.e2() + A.c[3 + 1 * 5] * B.e3() + A.c[4 + 1 * 5] * B.ni(),
		A.c[0 + 2 * 5] * B.no() + A.c[1 + 2 * 5] * B.e1() + A.c[2 + 2 * 5] * B.e2() + A.c[3 + 2 * 5] * B.e3() + A.c[4 + 2 * 5] * B.ni(),
		A.c[0 + 3 * 5] * B.no() + A.c[1 + 3 * 5] * B.e1() + A.c[2 + 3 * 5] * B.e2() + A.c[3 + 3 * 5] * B.e3() + A.c[4 + 3 * 5] * B.ni(),
		A.c[0 + 4 * 5] * B.no() + A.c[1 + 4 * 5] * B.e1() + A.c[2 + 4 * 5] * B.e2() + A.c[3 + 4 * 5] * B.e3() + A.c[4 + 4 * 5] * B.ni());
}

inline c3ga::flatPoint matrixFPMultiply(const Mat4x4& A, const c3ga::flatPoint& B) {
	return c3ga::flatPoint(c3ga::flatPoint_e1ni_e2ni_e3ni_noni,
		A.c[0 + 0 * 4] * B.e1ni() + A.c[1 + 0 * 4] * B.e2ni() + A.c[2 + 0 * 4] * B.e3ni() + A.c[3 + 0 * 4] * B.noni(),
		A.c[0 + 1 * 4] * B.e1ni() + A.c[1 + 1 * 4] * B.e2ni() + A.c[2 + 1 * 4] * B.e3ni() + A.c[3 + 1 * 4] * B.noni(),
		A.c[0 + 2 * 4] * B.e1ni() + A.c[1 + 2 * 4] * B.e2ni() + A.c[2 + 2 * 4] * B.e3ni() + A.c[3 + 2 * 4] * B.noni(),
		A.c[0 + 3 * 4] * B.e1ni() + A.c[1 + 3 * 4] * B.e2ni() + A.c[2 + 3 * 4] * B.e3ni() + A.c[3 + 3 * 4] * B.noni());
}

inline c3ga::dualPlane matrixDualPMultiply(const Mat4x4& A, const c3ga::dualPlane& B) {
	return c3ga::dualPlane(c3ga::dualPlane_e1_e2_e3_ni,
		A.c[0 + 0 * 4] * B.e1() + A.c[1 + 0 * 4] * B.e2() + A.c[2 + 0 * 4] * B.e3() + A.c[3 + 0 * 4] * B.ni(),
		A.c[0 + 1 * 4] * B.e1() + A.c[1 + 1 * 4] * B.e2() + A.c[2 + 1 * 4] * B.e3() + A.c[3 + 1 * 4] * B.ni(),
		A.c[0 + 2 * 4] * B.e1() + A.c[1 + 2 * 4] * B.e2() + A.c[2 + 2 * 4] * B.e3() + A.c[3 + 2 * 4] * B.ni(),
		A.c[0 + 3 * 4] * B.e1() + A.c[1 + 3 * 4] * B.e2() + A.c[2 + 3 * 4] * B.e3() + A.c[3 + 3 * 4] * B.ni());
}


inline void normalizePlane(c3ga::plane &P) {
	float mag = (1 / sqrtf(P.e1e3noni() * P.e1e3noni() + P.e1e2noni()
		* P.e1e2noni() + P.e2e3noni() * P.e2e3noni()));
	P.m_c[0] *= mag; P.m_c[1] *= mag; P.m_c[2] *= mag; P.m_c[3] *= mag;
}

inline void normalizeFlatPoint(c3ga::flatPoint& P) {
	float mag = (1 / sqrtf(P.e2ni() * P.e2ni() + P.e3ni()
		* P.e3ni() + P.e1ni() * P.e1ni()));
	P.m_c[0] *= mag; P.m_c[1] *= mag; P.m_c[2] *= mag; P.m_c[3] *= mag;
}

inline c3ga::plane linearCombo(c3ga::plane P, float s,
							   c3ga::plane Q, float t) {
	return c3ga::plane(c3ga::plane_e1e2e3ni_e1e2noni_e1e3noni_e2e3noni,
		P.m_c[0] * s + Q.m_c[0] * t, P.m_c[1] * s + Q.m_c[1] * t,
		P.m_c[2] * s + Q.m_c[2] * t, P.m_c[3] * s + Q.m_c[3] * t);
}

inline c3ga::flatPoint linearCombo(c3ga::flatPoint P, float s,
	c3ga::flatPoint Q, float t) {
	return c3ga::flatPoint(c3ga::flatPoint_e1ni_e2ni_e3ni_noni,
		P.m_c[0] * s + Q.m_c[0] * t, P.m_c[1] * s + Q.m_c[1] * t,
		P.m_c[2] * s + Q.m_c[2] * t, P.m_c[3] * s + Q.m_c[3] * t);
}