#include "matrix.h"

Matrix Matrix::Perspective( float fov, float aspect, float _near, float _far ) {
	float yScale = 1.0f / tanf( TO_RADIAN( fov ) / 2 );
	float xScale = yScale / aspect;
	float nearmfar = _near - _far;
	float projMatrixValue[ 16 ] = {
	    xScale,
	    0,
	    0,
	    0,
	    0,
	    yScale,
	    0,
	    0,
	    0,
	    0,
	    ( _far + _near ) / ( _near - _far ),
	    ( 2.0f * _far * _near ) / ( _near - _far ),
	    0,
	    0,
	    -1.0f,
	    0,
	};
	return Matrix( projMatrixValue );
}

Matrix Matrix::Orthographic( float _near, float _far, float l, float r, float t, float b ) {
	float projMatrixValue[ 16 ] = {
	    2.0f / ( r - l ),
	    0,
	    0,
	    -( r + l ) / ( r - l ),
	    0,
	    2.0f / ( t - b ),
	    0,
	    -( t + b ) / ( t - b ),
	    0,
	    0,
	    -2.0f / ( _far - _near ),
	    -( _far + _near ) / ( _far - _near ),
	    0,
	    0,
	    0,
	    1,
	};
	return Matrix( projMatrixValue );
}

void Matrix::EulerRotation( const Vec3 & rot ) {
	Matrix rotX, rotY, rotZ;
	float  c, s;

	c = cosf( rot[ 0 ] );
	s = sinf( rot[ 0 ] );
	rotX[ 5 ] = c;
	rotX[ 6 ] = -s;
	rotX[ 9 ] = s;
	rotX[ 10 ] = c;

	c = cosf( rot[ 1 ] );
	s = sinf( rot[ 1 ] );
	rotY[ 0 ] = c;
	rotY[ 2 ] = s;
	rotY[ 8 ] = -s;
	rotY[ 10 ] = c;

	c = cosf( rot[ 2 ] );
	s = sinf( rot[ 2 ] );
	rotZ[ 0 ] = c;
	rotZ[ 1 ] = -s;
	rotZ[ 4 ] = s;
	rotZ[ 5 ] = c;

	*this = *this * rotX * rotY * rotZ;
}

Matrix & Matrix::operator*=( float a ) {
	for ( int i = 0; i < 16; i++ ) {
		v[ i ] *= a;
	}
	return *this;
}

Matrix Matrix::operator*( const Matrix & m ) const {
	Matrix res;
	res.Zero();
	for ( int c = 0; c < 4; c++ ) {
		for ( int r = 0; r < 4; r++ ) {
			for ( int k = 0; k < 4; k++ ) {
				res.At( c, r ) += At( k, r ) * m.At( c, k );
			}
		}
	}
	return res;
}

Matrix Matrix::operator*( float a ) const {
	Matrix res;
	for ( int i = 0; i < 16; i++ ) {
		res.v[ i ] = v[ i ] * a;
	}
	return res;
}

Vec3 Matrix::operator*( const Vec3 & v ) const {
	Vec3 res = {
	    At( 0, 0 ) * v.x + At( 1, 0 ) * v.y + At( 2, 0 ) * v.z + At( 3, 0 ) * 1.0f,
	    At( 0, 1 ) * v.x + At( 1, 1 ) * v.y + At( 2, 1 ) * v.z + At( 3, 1 ) * 1.0f,
	    At( 0, 2 ) * v.x + At( 1, 2 ) * v.y + At( 2, 2 ) * v.z + At( 3, 2 ) * 1.0f,
	};
	return res;
}

Vec2 Matrix::operator*( const Vec2 & v ) const {
	Vec3 v3( v, 0.0f );
	return (*this * v3).ToVec2();
}

Matrix Matrix::operator+( const Matrix & rhs ) const {
	Matrix res;
	for ( int i = 0; i < 16; i++ ) {
		res.v[ i ] = v[ i ] + rhs.v[ i ];
	}
	return res;
}
